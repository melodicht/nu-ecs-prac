#pragma once

#include "math/skl_math_consts.h"

#include <webgpu/webgpu.h>

#include <glm/fwd.hpp>

// >>> Represents helper objects <<<
#pragma region misc types
// Simply combines a single texture and texture view
// Does not handle the release of the textures on its own
struct WGPUBackendTexture {
    WGPUTexture m_texture;
    WGPUTextureView m_textureView;
};

// Wraps around the core parts of WGPU to allow core of WGPU to be released last
struct WGPUCore {
    WGPUDevice m_device{ };
    WGPUInstance m_instance{ };

    ~WGPUCore() {
        wgpuDeviceRelease(m_device);
        wgpuInstanceRelease(m_instance);
    }
};

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
#pragma endregion

// >>> Represents types meant to be plugged directly into the buffer <<<
#pragma region cpu->gpu types

// Represents the location of a single texture within a texture atlas
// TODO: Likely going to be implemented with depth textures of point lights
// struct WGPUBackendTextureAtlasIdx {
//     u32 m_x;
//     u32 m_y;
//     u32 m_width;
//     u32 m_height;
// };

// Represents the transformation data of the camera
struct WGPUBackendCameraData
{
    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::vec3 m_pos;
    u32 m_filler; // ensures 144 byte size
};

// Represents a instance of a mesh
struct WGPUBackendObjectData {
    glm::mat4 m_model;
    glm::vec4 m_color;
};

// Represents a single shadowed directional light
struct WGPUBackendDynamicShadowedDirLightData {
    glm::vec3 m_direction;
    f32 m_intensity;
    glm::vec3 m_lightColor;
    u32 m_shadowMapIdxStart;
    u32 m_lightSpaceIdxStart;
    u32 m_lightCascadeCount;
};
#pragma endregion