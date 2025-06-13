#pragma once

#include "math/math_consts.h"
#include "render_types.h"

#include <SDL3/SDL.h>

#include <map>

// Common interface between renderers for systems to call.
// The interfaces take in descriptor objects in order to allow for 
// updates to the inputs of the interface without updates of everything that uses the interface .
// Assumes that a SDL3 surface is being drawn upon.

// Get the flags that should be added onto the SDL window creation to support this backend
SDL_WindowFlags GetRenderWindowFlags();

// Sets a SDL window to draw to and initializes the back end
struct RenderInitDescriptor {
    // Shared 
    SDL_Window *window;
    u32 startWidth;
    u32 startHeight;

    // Vulkan Specific 

    // WGPU Specific
};
void InitRenderer(RenderInitDescriptor& desc);

// Set up the render pipelines
struct RenderPipelineInitDescriptor {
    // Shared 

    // Vulkan Specific
    u32 numCascades;

    // WGPU Specific
};
void InitPipelines(RenderPipelineInitDescriptor& desc);

// Moves a mesh to the GPU,
// Returns a uint that represents the mesh's ID
struct RenderUploadMeshDescriptor {
    // Shared 
    Vertex* vertData;
    u32* idxData;
    u32 vertSize;
    u32 idxSize;

    // Vulkan Specific

    // WGPU Specific
};
u32 UploadMesh(RenderUploadMeshDescriptor& desc);

// Destroy the mesh at the given MeshID
struct RenderDestroyMeshDescriptor {
    // Shared 
    u32 meshID;

    // Vulkan Specific

    // WGPU Specific
};
void DestroyMesh(RenderDestroyMeshDescriptor& desc);

struct RenderAddCameraDescriptor {
    // Shared

    // Vulkan Specific
    u32 viewCount;

    // WGPU Specific
};
CameraID AddCamera(RenderAddCameraDescriptor& desc);

// Represents the information needed to render a single frame on any renderer
struct RenderFrameState {
    // Shared
    CameraData mainCam;
    std::vector<ObjectData> objData;
    std::map<u32, u32> meshCounts;

    // Vulkan Specific

    // WGPU Specific
};
// Renders a frame using the supplied render state
// The driving function of the entire renderer.
void RenderUpdate(RenderFrameState& state);