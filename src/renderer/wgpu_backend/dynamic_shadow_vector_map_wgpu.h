#pragma once

#include "math/skl_math_consts.h"

#include <vector>

// Stores shadowed light source information.
// Allows for shadows to be called and updated with ShadowIds while keeping information in a 
// array memory layout so that light data can be directly copied into GPU.
template <typename GPUType, typename CPUType, typename... ConversionArgs >
class WGPUBackendDynamicShadowVectorMap {
private:
    // The shadowId at a idx should correspond to a GPUType at the same index.
    std::vector<u32> shadowIds{ };
    std::vector<GPUType> shadowData{ };

protected:
    // Extracts ShadowId from vector map types
    virtual u32 GetCPUShadowId(const CPUType& cpuType) = 0;
    
    // Each index at cpuType should correspond to a index of a gpu type in cpuToGPUIndices that points to a GPUType in output with the same shadowId.
    virtual void Convert(std::vector<CPUType>& cpuType, std::vector<int>& cpuToGPUIndices,std::vector<GPUType>& output, ConversionArgs... args) = 0;

private:
    // A comparison function meant to allow for sorting of cpu types based on shadowId from least to greatest
    bool CPUShadowIdCompare (const CPUType& a, const CPUType& b) {
        return GetCPUShadowId(a) < GetCPUShadowId(b);
    }

public:
    WGPUBackendDynamicShadowVectorMap() { }
    virtual ~WGPUBackendDynamicShadowVectorMap() { }

    // This makes the assumption that the newest shadowIds added is the largest so that there does not need to be any process of sorting
    void PushBack(const CPUType& shadow) {
        shadowIds.push_back(GetCPUShadowId(shadow));
        // Creates a dummy GPUType at the back to be overwritten later
        shadowData.emplace_back();
    }

    // Assumes that no CPUType will have a shadowID not already registered or was removed.
    // If registered shadow id is not presented, the shadow will simply not update.
    void Update(std::vector<CPUType>& shadowUpdate, ConversionArgs... args) {
        std::sort(shadowUpdate.begin(), shadowUpdate.end(), CPUShadowIdCompare);

        std::vector<int> indices;

        auto gpuIdsIter = shadowIds.begin();
        auto shadowIter = shadowUpdate.begin()
        while (shadowIter != shadowUpdate.end() && gpuIdsIter != shadowIds.end()) {
            if (*gpuIdsIter == GetCPUShadowId(*shadowIter)) {
                indices.push_back(*gpuIndicesIter);
                shadowIter++;
            }
            gpuIdsIter++;
        }

        assert(indices.size() == shadowUpdate.size());
        Convert(shadowUpdate, indices, shadowData, args);
    }

    const std::vector<GPUType>& GetShadowData() {
        return shadowData;
    }
};

#include "renderer/wgpu_backend/render_types_wgpu.h"
#include "renderer/render_backend.h"

template <size_t CascadeSize>
class WGPUBackendDirectionalDynamicShadowMap : public
    WGPUBackendDynamicShadowVectorMap<
        WGPUBackendDynamicShadowedDirLightData<CascadeSize>,
        DirLightRenderInfo, 
        const glm::mat4x4& camView,
        float camAspect,
        float camFov, 
        float camNear, 
        float camFar> {
protected:
    u32 GetCPUShadowId(const DirLightRenderInfo& cpuType) override final {
        return cpuType.shadowId;
    }

    void Convert(
        std::vector<DirLightRenderInfo>& cpuType,
        std::vector<int>& cpuToGPUIndices,
        std::vector<WGPUBackendDynamicShadowedDirLightData>& output,
        const glm::mat4x4& camView,
        float camAspect,
        float camFov, 
        float camNear, 
        float camFar) override final {

        // Inserts non light space data into GPU data
        for (int cpuIter = 0; cpuIter < cpuType.size() ; cpuIter) {
            output[cpuToGPUIndices].m_intensity = cpuType[cpuIter].intensity;
            output[cpuToGPUIndices].m_lightColor = cpuType[cpuIter].color;
        }

        // Inserts light space matrix data into GPU data

        // TODO: Implement custom functionality for cascade ratios
        std::vector<glm::mat4x4> m_lightSpaces;
        std::array<int, CascadeSize> cascadeRatios = {0.25, 0.25, 0.25, 0.25};
        const float camNearFarDiff = camFar - camNear;
        float currentCascadeLength = camNear;
        for (int cascadeIterator = 0; cascadeIterator < CascadeSize; cascadeIterator++)
        {
            currentCascadeLength = cascadeRatios[cascadeIterator] * camNearFarDiff;
            glm::mat4 subProj = glm::perspective(glm::radians(camFov), camAspect,
                                                    camNear, camNear + currentCascadeLength);

            std::vector<glm::vec4> corners = getFrustumCorners(subProj, camView);
            for (int cpuIter = 0; cpuIter < cpuType.size() ; cpuIter++) {
                f32 minX = std::numeric_limits<f32>::max();
                f32 maxX = std::numeric_limits<f32>::lowest();
                f32 minY = std::numeric_limits<f32>::max();
                f32 maxY = std::numeric_limits<f32>::lowest();
                f32 maxZ = std::numeric_limits<f32>::lowest();

                for (const glm::vec4& v : corners) {
                    const glm::vec4 trf = v * cpuType[cpuIter].viewSpace;
                    minX = std::min(minX, trf.x);
                    maxX = std::max(maxX, trf.x);
                    minY = std::min(minY, trf.y);
                    maxY = std::max(maxY, trf.y);
                    maxZ = std::max(maxZ, trf.z);
                }

                // TODO: Find more specific method for determining maxZ size
                glm::mat4 dirProj = glm::ortho(minX, maxX, minY, maxY, maxZ - camFar, maxZ);  
                
                // Inserts dir light into gpu vector   
                output[cpuToGPUIndices].m_lightSpaces[cascadeIterator];
            }
        }
    }
};