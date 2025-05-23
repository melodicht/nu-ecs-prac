#pragma once

#include "render_types.h"
#include "math/math_consts.h"
#include "asset_types.h"

#include <SDL3/SDL.h>

#include <cstdint>

// Common interface between renderers for systems to call
// Assumes that a SDL3 surface is being drawn upon

// Get the flags that should be added onto the SDL window creation to support this backend
SDL_WindowFlags GetRenderWindowFlags();

// Sets a SDL window to draw to and initializes the back end
void InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight);

// Moves a mesh to the GPU,
// Returns a uint that represents the mesh's ID
MeshID UploadMesh(u32 vertCount, Vertex* vertices, u32 indexCount, u32* indices);
MeshID UploadMesh(MeshAsset &asset);

// Create a grayscale depth texture that can be used as a depth target
// and can also be sampled from shaders
TextureID CreateDepthTexture(u32 width, u32 height);

// Destroy the texture at the given TextureID
void DestroyTexture(TextureID textureID);

// Destroy the mesh at the given MeshID
void DestroyMesh(MeshID meshID);

// Initialize the frame and begin recording rendering commands
bool InitFrame();

// Begin a depth only rendering pass onto the screen depth image
// cullMode specifies the face culling mode to use for this pass
// depthBias specifies whether to apply a bias to the depth test during this pass (to solve shadow acne)
void BeginDepthPass(CullMode cullMode, bool depthBias);

// Begin a depth only rendering pass onto the given depth texture
void BeginDepthPass(TextureID target, CullMode cullMode, bool depthBias);

// Begin a color rendering pass
// cullMode specifies the face culling mode to use for this pass
// depthBias specifies whether to apply depth bias in this pass
void BeginColorPass(CullMode cullMode);

// End the current rendering pass
void EndPass();

// Draw the current ImGui frame onto the rendered image
void DrawImGui();

// Set the camera settings and transformation to use for rendering
void SetCamera(u32 id, glm::mat4 view, glm::mat4 proj, glm::vec3 pos);

// Set scene directional light information to use for rendering
void SetDirLight(glm::mat4 lightSpace, glm::vec3 lightDir, TextureID texture);

// Set the mesh currently being rendered to
void SetMesh(MeshID meshID);

// Send the object data of the models to render
void SendObjectData(std::vector<ObjectData>& objects);

// End the frame and present it to the screen
void EndFrame();

// Draw multiple objects to the screen whose object data starts at startIndex
// in the most recently provided object data
void DrawObjects(int count, int startIndex);