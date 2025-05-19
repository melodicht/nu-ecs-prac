#pragma once

#include "render_types.h"
#include "math/math_consts.h"
#include "asset_types.h"

#include <SDL3/SDL.h>

#include <cstdint>

// Common interface between renderers for systems to call
// Assumes that a SDL3 surface is being drawn upon

SDL_WindowFlags GetRenderWindowFlags();

// Sets a SDL window to draw to and initializes the back end
void InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight);

// Moves mesh to the GPU,
// Returns a uint that represents the mesh's ID
MeshID UploadMesh(u32 vertCount, Vertex* vertices, u32 indexCount, u32* indices);
MeshID UploadMesh(MeshAsset &asset);

// Takes in a mesh ID and represents
void DestroyMesh(MeshID meshID);

// Initialize the frame and begin recording rendering commands
bool InitFrame();

// Begin a depth only rendering pass
// cullMode specifies the face culling mode to use for this pass
// depthBias specifies whether to apply a bias to the depth test during this pass (to solve shadow acne)
void BeginDepthPass(CullMode cullMode, bool depthBias);

// Begin a color rendering pass
// cullMode specifies the face culling mode to use for this pass
// depthBias specifies whether to apply depth bias in this pass
void BeginColorPass(CullMode cullMode);

// End a rendering pass
void EndPass();

// Draw ImGui content
void DrawImGui();

// Sets the view of a camera
void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos);

// Sets the mesh currently being rendered to
void SetMesh(MeshID meshID);

// Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
void SendObjectData(std::vector<ObjectData>& objects);

// End the frame and present it to the screen
void EndFrame();

// Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
void DrawObjects(int count, int startIndex);