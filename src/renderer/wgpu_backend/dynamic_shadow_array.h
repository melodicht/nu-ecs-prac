#pragma once 

#include "math/skl_math_consts.h"
#include "renderer/wgpu_backend/utils_wgpu.h"
#include "renderer/wgpu_backend/render_types_wgpu.h"
#include "renderer/wgpu_backend/bind_group_wgpu.h"

#include <webgpu/webgpu.h>

#include <string>
#include <vector>

// Encapsulates a single Shadow map texture array
template <typename GPUShadowType>
class WGPUBackendBaseDynamicShadowMapArray : WGPUBackendBindGroup::IWGPUBackendUniformEntry {
private:
    std::vector<std::reference_wrapper<WGPUBackendBindGroup>> m_bindGroups;
    std::vector<WGPUTextureView> m_arrayLayerViews;
    std::vector<bool> m_usedSlots;
    WGPUTextureView m_wholeTextureDataView;
    WGPUTexture m_textureData;
    std::string m_label;
    // Unused slots less than m_arraySize
    WGPUBindGroupEntry m_currentBindGroupEntry;
    u32 m_arrayLayerWidth;
    u32 m_arrayLayerHeight;
    u16 m_arraySize;
    u16 m_arrayAllocatedSize;
    u16 m_arrayMaxAllocatedSize;

protected:
    u16 GenerateNewAllocatedSize (u16 newArraySize) {

    }

    virtual u16 GetShadowLayerSize(const GPUShadowType& shadow) = 0;
    virtual void SetShadowLayerStart(GPUShadowType& shadow) = 0;

public:
    WGPUBackendBaseDynamicShadowMapArray();

    virtual ~WGPUBackendBaseDynamicShadowMapArray();

        void Init(const WGPUDevice& device, u32 arrayLayerWidth, u32 arrayLayerHeight, u32 maxTextureDepth, std::string label, u16 binding);

    // Sets shadow to point to correct starting index in array
    void Register(const WGPUDevice& device, const WGPUQueue& queue, WGPUBackendDynamicShadowedDirLightData& shadow);
    // Removes selected shadow from array
    void Unregister(const WGPUBackendDynamicShadowedDirLightData& shadow);

    // Used to update bind group on shadow texture resizing
    WGPUBindGroupEntry GetEntry() override;
    void RegisterBindGroup(WGPUBackendBindGroup& bindGroup) override;

    WGPUTextureView GetView(u16 shadowIndex);
};

// Encapsulates a single depth texture array meant to represent all dynamic directional shadows in a scene, able to move around and disappear
class WGPUBackendDynamicDirShadowArray : WGPUBackendBindGroup::IWGPUBackendUniformEntry {
private:
    std::vector<std::reference_wrapper<WGPUBackendBindGroup>> m_bindGroups;
    std::vector<WGPUTextureView> m_arrayLayerViews;
    std::vector<bool> m_usedSlots;
    WGPUTextureView m_wholeTextureDataView;
    WGPUTexture m_textureData;
    std::string m_label;
    // Unused slots less than m_arraySize
    WGPUBindGroupEntry m_currentBindGroupEntry;
    u32 m_arrayLayerWidth;
    u32 m_arrayLayerHeight;
    u16 m_arraySize;
    u16 m_arrayAllocatedSize;
    u16 m_arrayMaxAllocatedSize;

    // Continually doubles allocated size until allocation can fit given new arraySize.
    // Doesn't actually edit arraySize however.
    void ResizeTexture(const WGPUDevice& device, const WGPUQueue& queue, u32 newArraySize);

public:
    WGPUBackendDynamicDirShadowArray();

    ~WGPUBackendDynamicDirShadowArray();

    void Init(const WGPUDevice& device, u32 arrayLayerWidth, u32 arrayLayerHeight, u32 maxTextureDepth, std::string label, u16 binding);

    // Sets shadow to point to correct starting index in array
    void Register(const WGPUDevice& device, const WGPUQueue& queue, WGPUBackendDynamicShadowedDirLightData& shadow);
    // Removes selected shadow from array
    void Unregister(const WGPUBackendDynamicShadowedDirLightData& shadow);

    // Used to update bind group on shadow texture resizing
    WGPUBindGroupEntry GetEntry() override;
    void RegisterBindGroup(WGPUBackendBindGroup& bindGroup) override;

    WGPUTextureView GetView(u16 shadowIndex);
};