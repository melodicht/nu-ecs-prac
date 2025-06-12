#pragma once

#include "game.h"
#include "ecs.h"
#include "render_types.h"
#include "asset_types.h"

#include <SDL3/SDL.h>

#include <cstdint>

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
    MeshAsset meshAsset;

    // Vulkan Specific

    // WGPU Specific
};
MeshID UploadMesh(RenderUploadMeshDescriptor& desc);

// Destroy the mesh at the given MeshID
struct RenderDestroyMeshDescriptor {
    // Shared 
    MeshID meshID;

    // Vulkan Specific

    // WGPU Specific
};
void DestroyMesh(RenderDestroyMeshDescriptor& desc);

// The main update function of the renderer 
struct RenderUpdateDescriptor {
    // Shared 
    Scene *scene;
    u32 screenWidth;
    u32 screenHeight;

    // Vulkan Specific
    f32 deltaTime;

    // WGPU Specific
};
void RenderUpdate(RenderUpdateDescriptor& desc);

struct RenderAddCameraDescriptor {
    // Shared

    // Vulkan Specific
    u32 viewCount;

    // WGPU Specific
};
CameraID AddCamera(RenderAddCameraDescriptor& desc);

// Represents information plugged into the configuration of
// Done in objects so that changes in needs of initialization isn't met with need for change in interface.

// DEPRECATED COMMANDS
// // Create a grayscale depth texture that can be used as a depth target
// // and can also be sampled from shaders
// TextureID CreateDepthTexture(u32 width, u32 height);

// // Create a depth array texture with the given dimensions and number of layers
// TextureID CreateDepthArray(u32 width, u32 height, u32 layers);

// // Destroy the texture at the given TextureID
// void DestroyTexture(TextureID textureID);

// // Destroy the mesh at the given MeshID
// void DestroyMesh(MeshID meshID);

// // Add a new camera to the scene. You need multiple cameras
// // if you want to render multiple views in the same frame.
// CameraID AddCamera(u32 viewCount);

// // Initialize the frame and begin recording rendering commands
// bool InitFrame();

// // Begin a depth only rendering pass onto the screen depth image
// // cullMode specifies the face culling mode to use for this pass
// // depthBias specifies whether to apply a bias to the depth test during this pass (to solve shadow acne)
// void BeginDepthPass(CullMode cullMode);

// // Begin a shadow depth pass onto the given texture
// void BeginShadowPass(TextureID target, CullMode cullMode);

// // Begin a multiview shadow depth pass onto the given array texture
// void BeginCascadedPass(TextureID target, CullMode cullMode);

// // Begin a color rendering pass
// // cullMode specifies the face culling mode to use for this pass
// // depthBias specifies whether to apply depth bias in this pass
// void BeginColorPass(CullMode cullMode);

// // End the current rendering pass
// void EndPass();

// // Draw the current ImGui frame onto the rendered image
// void DrawImGui();

// // Set the camera to use for rendering with the given ID
// void SetCamera(CameraID id);

// // Update the currently selected camera. viewCount must be equal to the number of views that the selected camera has.
// void UpdateCamera(u32 viewCount, CameraData* views);

// // Set scene directional light information to use for rendering
// void SetDirLight(LightCascade* cascades, glm::vec3 lightDir, TextureID texture);

// // Set the mesh currently being rendered to
// void SetMesh(MeshID meshID);

// // Send the object data of the models to render
// void SendObjectData(std::vector<ObjectData>& objects);

// // End the frame and present it to the screen
// void EndFrame();

// // Draw multiple objects to the screen whose object data starts at startIndex
// // in the most recently provided object data
// void DrawObjects(int count, int startIndex);