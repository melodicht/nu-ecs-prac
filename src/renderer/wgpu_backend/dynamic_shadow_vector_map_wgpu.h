#pragma once

#include "skl_logger.h"
#include "math/skl_math_utils.h"

#include <vector>
#include <utility>
#include <functional>

// Stores shadowed light source information.
// Allows for shadows to be called and updated with lightIds while keeping information in a 
// array memory layout so that light data can be directly copied into GPU.
template <typename GPUType, typename CPUType, typename... ConversionArgs >
class WGPUBackendDynamicShadowVectorMap {
private:
    // The lightID at a idx should correspond to a GPUType at the same index.
    // Also lightIds should always be sorted from least to greatest
    std::vector<LightID> lightIds{ };
    std::vector<GPUType> shadowData{ };

protected:
    // Extracts lightID from vector map types
    virtual LightID GetCPULightID(const CPUType& cpuType) = 0;
    
    // Each index at cpuType should correspond to a index of a gpu type in cpuToGPUIndices that points to a GPUType in output with the same lightID.
    virtual void Convert(std::vector<CPUType>& cpuType, std::vector<int>& cpuToGPUIndices,std::vector<GPUType>& output, ConversionArgs... args) = 0;

public:
    WGPUBackendDynamicShadowVectorMap() { }
    virtual ~WGPUBackendDynamicShadowVectorMap() { }

    // This makes the assumption that the newest lightId added is the largest so that there does not need to be any process of sorting
    void PushBack(LightID id) {
        lightIds.push_back(id);
        // Creates a dummy GPUType at the back to be overwritten later
        shadowData.emplace_back();
    }

    void Erase(LightID lightId) {
        // Finds lightIter with assumption of being sorted
        std::vector<LightID>::iterator lightIter = std::lower_bound(lightIds.begin(), lightIds.end(), lightId, std::less<u32>());

        if (lightIter != lightIds.end() && *lightIter == lightId) {
            shadowData.erase(shadowData.begin() + std::distance(lightIds.begin(), lightIter));
            lightIds.erase(lightIter);    
        }
        else {
            LOG_ERROR("Light ID that was erased never existed or lied in unsorted vector");
        }
    }

    // This functions works under the assumption that no unregistered shadows  will come in through shadowUpdate.
    // However if a registered shadow id is not present in shadowUpdate, the shadow will simply not update.
    void Update(std::vector<CPUType>& shadowUpdate, ConversionArgs... args) {
        std::sort(shadowUpdate.begin(), shadowUpdate.end(), [this](const CPUType& a, const CPUType& b) {
            return this->GetCPULightID(a) < this->GetCPULightID(b);
        });

        std::vector<int> indices;

        auto gpuIdsIter = lightIds.begin();
        auto shadowIter = shadowUpdate.begin();
        while (shadowIter != shadowUpdate.end() && gpuIdsIter != lightIds.end()) {
            if (*gpuIdsIter == GetCPULightID(*shadowIter)) {
                indices.push_back(std::distance(lightIds.begin(), gpuIdsIter));
                shadowIter++;
            }
            gpuIdsIter++;
        }

        assert(indices.size() == shadowUpdate.size());
        Convert(shadowUpdate, indices, shadowData, args...);
    }

    const std::vector<GPUType>& GetShadowData() {
        return shadowData;
    }
};

#include "renderer/wgpu_backend/render_types_wgpu.h"
#include "renderer/render_backend.h"

template <size_t CascadeSize>
class WGPUBackendDirectionalDynamicShadowMap : public WGPUBackendDynamicShadowVectorMap<
        WGPUBackendDynamicShadowedDirLightData<CascadeSize>,
        DirLightRenderInfo, 
        const glm::mat4x4*,
        const float,
        const float, 
        const float, 
        const float> {
protected:
    LightID GetCPULightID(const DirLightRenderInfo& cpuType) override final {
        return cpuType.lightID;
    }

    void Convert(
        std::vector<DirLightRenderInfo>& cpuType,
        std::vector<int>& cpuToGPUIndices,
        std::vector<WGPUBackendDynamicShadowedDirLightData<CascadeSize>>& output,
        const glm::mat4x4* camView,
        const float camAspect,
        const float camFov, 
        const float camNear, 
        const float camFar) override final {

        // Inserts non light space data into GPU data
        std::vector<glm::mat4x4> lightViews;
        lightViews.reserve(cpuType.size());
        for (int cpuIter = 0; cpuIter < cpuType.size() ; cpuIter++) {
            output[cpuToGPUIndices[cpuIter]].m_direction = glm::vec4(GetForwardVector(&cpuType[cpuIter].transform),1);
            output[cpuToGPUIndices[cpuIter]].m_diffuse = glm::vec4(cpuType[cpuIter].diffuse, 1);
            output[cpuToGPUIndices[cpuIter]].m_specular = glm::vec4(cpuType[cpuIter].specular, 1);
            lightViews.push_back(GetViewMatrix(&cpuType[cpuIter].transform));
        }

        // Inserts light space matrix data into GPU data

        // TODO: Implement custom functionality for cascade ratios
        std::vector<glm::mat4x4> m_lightSpaces;
            std::array<std::array<float, 2>, CascadeSize> cascadeRatios = {{{{0.0f, 0.3f}}, {{0.25f, 0.55f}}, {{0.5f, 0.8f}} , {{0.75f, 1.00f}}}};
        
        const float camNearFarDiff = camFar - camNear;
        float currentCascadeEnd = camNear;
        float currentCascadeStart = camNear;
        for (int cascadeIterator = 0; cascadeIterator < CascadeSize; cascadeIterator++)
        {
            currentCascadeStart = cascadeRatios[cascadeIterator][0] * camNearFarDiff;
            currentCascadeEnd = cascadeRatios[cascadeIterator][1] * camNearFarDiff;
            glm::mat4 subProj = glm::perspective(glm::radians(camFov), camAspect,
                                                    camNear + currentCascadeStart, camNear + currentCascadeEnd);

            std::vector<glm::vec4> corners = GetFrustumCorners(subProj, *camView);
            for (int cpuIter = 0; cpuIter < cpuType.size() ; cpuIter++) {
                f32 minX = std::numeric_limits<f32>::max();
                f32 maxX = std::numeric_limits<f32>::lowest();
                f32 minY = std::numeric_limits<f32>::max();
                f32 maxY = std::numeric_limits<f32>::lowest();
                f32 maxZ = std::numeric_limits<f32>::lowest();

                for (const glm::vec4& v : corners) {
                    const glm::vec4 trf = v * lightViews[cpuIter];
                    minX = std::min(minX, trf.x);
                    maxX = std::max(maxX, trf.x);
                    minY = std::min(minY, trf.y);
                    maxY = std::max(maxY, trf.y);
                    maxZ = std::max(maxZ, trf.z);
                }

                // TODO: Find more specific method for determining maxZ size
                glm::mat4 dirProj = glm::ortho(minX, maxX, minY, maxY, maxZ - camFar, maxZ);  
                
                // Inserts dir light into gpu vector   
                output[cpuToGPUIndices[cpuIter]].m_lightSpaces[cascadeIterator] = dirProj * lightViews[cpuIter];
            }
        }
    }
};
