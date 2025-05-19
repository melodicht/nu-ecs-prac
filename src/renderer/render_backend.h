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
u32 UploadMesh(u32 vertCount, Vertex* vertices, u32 indexCount, u32* indices);
u32 UploadMesh(MeshAsset &asset);

// Takes in a mesh ID and represents
void DestroyMesh(u32 meshID);

// Initialize the frame and begin recording rendering commands
bool InitFrame();

// Begin a depth only rendering pass
void BeginDepthPass();

// End a depth only rendering pass
void EndDepthPass();

// Begin a color rendering pass
void BeginColorPass();

// End a color rendering pass
void EndColorPass();

// Sets the view of a camera
void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos);

// Sets the mesh currently being rendered to
void SetMesh(u32 meshID);

// Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
void SendObjectData(std::vector<ObjectData>& objects);

// End the frame and present it to the screen
void EndFrame();

// Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
void DrawObjects(int count, int startIndex);