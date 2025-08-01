#include "renderer/wgpu_backend/dynamic_shadow_array.h"

#include <algorithm>

void WGPUBackendDynamicDirShadowArray::ResizeTexture(const WGPUDevice& device, const WGPUQueue& queue, u32 newArraySize) {
    assert(newArraySize <= m_arrayMaxAllocatedSize);

    m_arrayAllocatedSize = newArraySize;

    // Creates new texture and destroys old one in gpu
    WGPUTextureDescriptor newTextureDesc {
        .nextInChain = nullptr,
        .label = WGPUBackendUtils::wgpuStr(m_label.data()),
        .usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_StorageBinding,
        .dimension = WGPUTextureDimension_2D,
        .size = {
            .width = m_arrayLayerWidth,
            .height = m_arrayLayerHeight,
            .depthOrArrayLayers = m_arrayAllocatedSize,
        },
        .format = WGPUTextureFormat_Depth32Float,
        .mipLevelCount = 0,
        .sampleCount = 1
    };

    // No need to copy since all data gets cleared on being rendered to anyways
    wgpuTextureDestroy(m_textureData);
    m_textureData = wgpuDeviceCreateTexture(device, &newTextureDesc);

    // Destroys and recreates all of the previous texture views
    WGPUTextureViewDescriptor newTextureViewDesc {
        .nextInChain = nullptr,
        .label = WGPUBackendUtils::wgpuStr("Dynamic Directional Shadowed Light Texture View"),
        .format = WGPUTextureFormat_Depth32Float,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1, 
        .baseArrayLayer = 0,
        .arrayLayerCount = m_arrayAllocatedSize,
        .aspect = WGPUTextureAspect_DepthOnly,
        .usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_StorageBinding
    };

    wgpuTextureViewRelease(m_wholeTextureDataView);
    m_wholeTextureDataView = wgpuTextureCreateView(m_textureData, &newTextureViewDesc);

    newTextureViewDesc.arrayLayerCount = 1;
    newTextureViewDesc.label = WGPUBackendUtils::wgpuStr("Dynamic Directional Shadowed Light Layered Texture View");

    const size_t viewAmount = m_arrayLayerViews.size();
    for (u16 viewIdx = 0 ; viewIdx < viewAmount ; viewIdx++) {
        wgpuTextureViewRelease(m_arrayLayerViews[viewIdx]);
        newTextureViewDesc.baseArrayLayer = viewIdx;
        m_arrayLayerViews[viewIdx] = wgpuTextureCreateView(m_textureData, &newTextureViewDesc);
    }

    for (u16 nextIdx = viewAmount ; viewAmount < m_arrayAllocatedSize ; nextIdx++) {
        newTextureViewDesc.baseArrayLayer = nextIdx;
        m_arrayLayerViews[nextIdx] = wgpuTextureCreateView(m_textureData, &newTextureViewDesc);
    }
}

WGPUBackendDynamicDirShadowArray::WGPUBackendDynamicDirShadowArray() : 
    m_bindGroups(),
    m_usedSlots({ false }),
    m_textureData(),
    m_label("un-inited"),
    m_currentBindGroupEntry(),
    m_arrayLayerWidth(0),
    m_arrayLayerHeight(0),
    m_arraySize(0),
    m_arrayAllocatedSize(1),
    m_arrayMaxAllocatedSize(0) { }
WGPUBackendDynamicDirShadowArray::~WGPUBackendDynamicDirShadowArray() {
    wgpuTextureDestroy(m_textureData);
}

void WGPUBackendDynamicDirShadowArray::Init(const WGPUDevice& device, u32 arrayLayerWidth, u32 arrayLayerHeight, u16 maxTextureArraySize, std::string label, u32 binding)
{
    m_label = label;
    m_arrayLayerWidth = arrayLayerWidth;
    m_arrayLayerHeight = arrayLayerHeight;
    m_arrayMaxAllocatedSize = maxTextureArraySize;

    // Creates empty shadow map
    WGPUTextureDescriptor textureDesc {
        .nextInChain = nullptr,
        .label = WGPUBackendUtils::wgpuStr("Dynamic Directional Shadowed Light Shadow Map"),
        .usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc | WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_StorageBinding,
        .dimension = WGPUTextureDimension_2D,
        .size = {
            .width = arrayLayerWidth,
            .height = arrayLayerHeight,
            .depthOrArrayLayers = m_arrayAllocatedSize  
        },
        .format = WGPUTextureFormat_Depth32Float,
        .mipLevelCount = 0,
        .sampleCount = 1,
        .viewFormatCount = 0,
        .viewFormats = nullptr
    };

    // Create views to link to shadow map
    m_textureData = wgpuDeviceCreateTexture(device, &textureDesc);
    
    const WGPUTextureViewDescriptor textureViewDesc {
        .nextInChain = nullptr,
        .label = WGPUBackendUtils::wgpuStr("Dynamic Directional Shadowed Light Texture View"),
        .format = WGPUTextureFormat_Depth32Float,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1, 
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = WGPUTextureAspect_DepthOnly,
        .usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc | WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_StorageBinding
    };

    m_wholeTextureDataView = wgpuTextureCreateView(m_textureData, &textureViewDesc);

    m_arrayLayerViews.push_back(wgpuTextureCreateView(m_textureData, &textureViewDesc));

    // Creates bind group entry
    m_currentBindGroupEntry = {
        .nextInChain = nullptr,
        .binding = binding,
        .buffer = nullptr,
        .offset = 0,
        .size = 0,
        .sampler = nullptr,
        .textureView = m_wholeTextureDataView
    };
};

void WGPUBackendDynamicDirShadowArray::Register(const WGPUDevice& device, const WGPUQueue& queue, WGPUBackendDynamicShadowedDirLightData& shadow) {
    // Checks if gap between current array size and allocated size can fit cascade size
    if (m_arrayAllocatedSize - m_arraySize > shadow.m_lightCascadeCount) {
        shadow.m_shadowMapIdxStart = m_arraySize;
        m_usedSlots.insert(m_usedSlots.end(), shadow.m_lightCascadeCount, true);
        m_arraySize += shadow.m_lightCascadeCount;
        return;
    }

    // Checks for any no shadows no longer being used
    u16 contiguousCounter = 0;
    for (u16 iter = 0  ; iter < m_usedSlots.size() ; iter++) {
        if (m_usedSlots[iter]) {
            contiguousCounter++;
            if (contiguousCounter >= shadow.m_lightCascadeCount) {
                shadow.m_shadowMapIdxStart = iter - (contiguousCounter - 1);
                std::fill(m_usedSlots.begin() + shadow.m_shadowMapIdxStart, m_usedSlots.begin() + shadow.m_shadowMapIdxStart + shadow.m_lightCascadeCount, true);
                return;
            }
        }
        else {
            contiguousCounter = 1;
        }
    }

    // It seems that all options have been exhausted, the shadow array needs to expand
    ResizeTexture(device, queue, m_arraySize + shadow.m_lightCascadeCount);
    shadow.m_shadowMapIdxStart = m_arraySize;
    m_usedSlots.insert(m_usedSlots.end(), shadow.m_lightCascadeCount, true);
    m_arraySize += shadow.m_lightCascadeCount;
    return;
}
void WGPUBackendDynamicDirShadowArray::Unregister(const WGPUBackendDynamicShadowedDirLightData& shadow) {
    std::fill(m_usedSlots.begin() + shadow.m_shadowMapIdxStart, m_usedSlots.begin() + shadow.m_shadowMapIdxStart + shadow.m_lightCascadeCount, false);
}

WGPUBindGroupEntry WGPUBackendDynamicDirShadowArray::GetEntry() {
    return m_currentBindGroupEntry;
}

void WGPUBackendDynamicDirShadowArray::RegisterBindGroup(WGPUBackendBindGroup& bindGroup) {
    m_bindGroups.push_back(bindGroup);
}

WGPUTextureView WGPUBackendDynamicDirShadowArray::GetView(u16 shadowIndex) {
    return m_arrayLayerViews[shadowIndex];
}
