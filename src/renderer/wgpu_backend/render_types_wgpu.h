#pragma once

#include "math/skl_math_types.h"

#include <webgpu/webgpu.h>

#include <glm/fwd.hpp>

// Represents location of a specified mesh within the WebGPU renderer
struct WGPUBackendMeshIdx {
    u32 m_baseIndex{ 0 };
    u32 m_baseVertex{ 0 };
    u32 m_indexCount{ 0 };
    u32 m_vertexCount{ 0 };

    WGPUBackendMeshIdx() : 
        m_baseIndex(),
        m_baseVertex(),
        m_indexCount(),
        m_vertexCount()
    {}

    WGPUBackendMeshIdx(u32 baseIndex, u32 baseVertex, u32 indexCount, u32 vertexCount) : 
        m_baseIndex(baseIndex),
        m_baseVertex(baseVertex),
        m_indexCount(indexCount),
        m_vertexCount(vertexCount)
    {}
};

// Simply combines a single texture and texture view
// Does not handle the release of the textures
struct WGPUBackendTexture {
    WGPUTexture m_texture;
    WGPUTextureView m_textureView;
};

// Represents a single shadowed directional light
struct WGPUBackendDynamicShadowedDirLight {
    glm::vec3 m_direction;
    glm::vec3 m_lightColor;
    f32 m_intensity;
    u32 m_lightSpaceIdxStart;
    u32 m_lightCascadeCount;
};