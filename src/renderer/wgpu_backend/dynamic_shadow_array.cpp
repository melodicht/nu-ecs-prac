#include "renderer/wgpu_backend/dynamic_shadow_array.h"

#include <algorithm>

u16 WGPUBackendBaseDynamicShadowMapArray::GenerateNewAllocatedSize (u16 newArraySize) {
    assert(newArraySize <= m_arrayMaxAllocatedSize);

    return newArraySize;
}

void WGPUBackendBaseDynamicShadowMapArray::ResizeTexture(const WGPUDevice& device, const WGPUQueue& queue, u16 newArraySize) {
    m_arrayAllocatedSize = GenerateNewAllocatedSize(newArraySize);

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
    WGPUTexture oldTexture = m_textureData;
    m_textureData = wgpuDeviceCreateTexture(device, &newTextureDesc);

    // Destroys and recreates all of the previous texture views
    WGPUTextureViewDescriptor newTextureViewDesc {
        .nextInChain = nullptr,
        .label = WGPUBackendUtils::wgpuStr(m_wholeViewLabel.data()),
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
    newTextureViewDesc.label = WGPUBackendUtils::wgpuStr(m_layerViewLabel.data());

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

    // Removes old texture now that all old texture views are gone
    wgpuTextureDestroy(oldTexture);
}

void WGPUBackendBaseDynamicShadowMapArray::Clear() {
    // Deallocate on WGPU Side
    for ( auto iter = m_arrayLayerViews.begin() ; iter != m_arrayLayerViews.end() ; iter++) {
        wgpuTextureViewRelease(*iter);
    }
    wgpuTextureViewRelease(m_wholeTextureDataView);
    wgpuTextureDestroy(m_textureData);

    // Resets to pre-init state
    m_bindGroups = {};
    m_textureData = {};
    m_label = {"un-inited"};
    m_wholeViewLabel = {"un-inited"};
    m_layerViewLabel = {"un-inited"};
    m_currentBindGroupEntry = {};
    m_arrayLayerWidth = {0};
    m_arrayLayerHeight = {0};
    m_arraySize = {0};
    m_arrayAllocatedSize = {1};
    m_arrayMaxAllocatedSize = {0};
    m_depthPerShadow = {0};
    m_inited = {false};
}

WGPUBackendBaseDynamicShadowMapArray::WGPUBackendBaseDynamicShadowMapArray() : 
m_bindGroups(),
m_textureData(),
m_label("un-inited"),
m_wholeViewLabel("un-inited"),
m_layerViewLabel("un-inited"),
m_currentBindGroupEntry(),
m_arrayLayerWidth(0),
m_arrayLayerHeight(0),
m_arraySize(0),
m_arrayAllocatedSize(1),
m_arrayMaxAllocatedSize(0),
m_depthPerShadow(0),
m_inited(false) { }

WGPUBackendBaseDynamicShadowMapArray::~WGPUBackendBaseDynamicShadowMapArray() {
    Clear();
}

void WGPUBackendBaseDynamicShadowMapArray::Init(
    const WGPUDevice& device, 
    u32 arrayLayerWidth, 
    u32 arrayLayerHeight, 
    u16 maxTextureDepth, 
    u16 depthPerShadow, 
    std::string label,
    std::string wholeViewLabel,
    std::string layerViewLabel, 
    u16 binding) {
    // TODO: Prevent this from running in final build
    assert( !m_inited );
    m_label = label;
    m_wholeViewLabel = wholeViewLabel;
    m_layerViewLabel= layerViewLabel;
    m_arrayLayerWidth = arrayLayerWidth;
    m_arrayLayerHeight = arrayLayerHeight;
    m_arrayMaxAllocatedSize = maxTextureDepth;
    m_depthPerShadow = depthPerShadow;
    m_inited = true;

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
}

void WGPUBackendBaseDynamicShadowMapArray::Reset(
    const WGPUDevice& device, 
    u32 arrayLayerWidth, 
    u32 arrayLayerHeight, 
    u16 maxTextureDepth, 
    u16 depthPerShadow, 
    std::string label,
    std::string wholeViewLabel,
    std::string layerViewLabel,
    u16 binding) {
    Clear();
    Init(device, arrayLayerWidth, arrayLayerHeight, maxTextureDepth, depthPerShadow, label, wholeViewLabel, layerViewLabel, binding);
}

void WGPUBackendBaseDynamicShadowMapArray::RegisterShadow(const WGPUDevice& device, const WGPUQueue& queue) {
    // Check gap between current array size and allocated size can fit cascade size
    if (m_arrayAllocatedSize - m_arraySize > m_depthPerShadow) {
        m_arraySize += m_depthPerShadow;
    }
    // If it seems that all options have been exhausted, the shadow array needs to expand allocated memory
    else {
        ResizeTexture(device, queue, m_arraySize + m_depthPerShadow);
        m_arraySize += m_depthPerShadow;
    }
}

void WGPUBackendBaseDynamicShadowMapArray::UnregisterShadow() {
    m_arraySize -= m_depthPerShadow;
}

WGPUBindGroupEntry WGPUBackendBaseDynamicShadowMapArray::GetEntry() {
    return m_currentBindGroupEntry;
}
void WGPUBackendBaseDynamicShadowMapArray::RegisterBindGroup(WGPUBackendBindGroup& bindGroup) {
    m_bindGroups.push_back(bindGroup);
}

WGPUTextureView WGPUBackendBaseDynamicShadowMapArray::GetView(u16 shadowIndex) {
    return m_arrayLayerViews[shadowIndex];
}

