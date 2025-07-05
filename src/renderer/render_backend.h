#pragma once

#include "math/skl_math_consts.h"
#include "render_types.h"

#include <SDL3/SDL.h>

#include <map>

// Common interface between renderers for systems to call.
// The interfaces take in Info objects in order to allow for 
// updates to the inputs of the interface without updates of everything that uses the interface .
// Assumes that a SDL3 surface is being drawn upon.

// Get the flags that should be added onto the SDL window creation to support this backend
SDL_WindowFlags GetRenderWindowFlags();

// Sets a SDL window to draw to and initializes the back end
struct RenderInitInfo {
    // Shared 
    SDL_Window *window;
    u32 startWidth;
    u32 startHeight;

    // Vulkan Specific 

    // WGPU Specific
};
void InitRenderer(RenderInitInfo& info);

// Set up the render pipelines
struct RenderPipelineInitInfo {
    // Shared 

    // Vulkan Specific
    u32 numCascades;

    // WGPU Specific
};
void InitPipelines(RenderPipelineInitInfo& info);

// Moves a mesh to the GPU,
// Returns a uint that represents the mesh's ID
struct RenderUploadMeshInfo {
    // Shared 
    Vertex* vertData;
    u32* idxData;
    u32 vertSize;
    u32 idxSize;

    // Vulkan Specific

    // WGPU Specific
};
u32 UploadMesh(RenderUploadMeshInfo& info);

// Destroy the mesh at the given MeshID
struct RenderDestroyMeshInfo {
    // Shared 
    u32 meshID;

    // Vulkan Specific

    // WGPU Specific
};
void DestroyMesh(RenderDestroyMeshInfo& info);

struct RenderAddCameraInfo {
    // Shared

    // Vulkan Specific
    u32 viewCount;

    // WGPU Specific
};
CameraID AddCamera(RenderAddCameraInfo& info);

struct MeshRenderInfo {
    // Shared
    glm::mat4 matrix;
    glm::vec3 rgbColor;
    MeshID mesh;

    // Vulkan Specific

    // WGPU Specific
};

struct DirLightRenderInfo {
    // Shared
    glm::mat4x4 viewSpace;
    glm::vec3 dir;
    u32 shadowID; 
    glm::vec3 color;
    f32 intensity;

    // Vulkan Specific 

    // WGPU Specific
};

// Represents the information needed to render a single frame on any renderer
struct RenderFrameInfo {
    // Shared
    CameraData mainCam;
    std::vector<MeshRenderInfo> &meshes;

    // Vulkan Specific

    // WGPU Specific
    std::vector<DirLightRenderInfo>& dirLights; // Currently i'm making the assumption that all dir lights are dynamic.
    float mainCamAspect;
    float mainCamFov;
    float mainCamNear;
    float mainCamFar;
};

// Renders a frame using the supplied render state
// The driving function of the entire renderer.
void RenderUpdate(RenderFrameInfo& info);