#pragma once

#include "render_types.h"
#include "asset_utils.cpp"

#include <cstdint>

// Common interface between renderers for systems to call
// Assumes that a SDL3 surface is being drawn upon
class IRenderBackend {
public:
    // Sets a SDL window to draw to and initializes the back end
    virtual void InitRenderer(SDL_Window *window) = 0;

    // Moves mesh to the GPU, 
    // Returns a uint that represents the mesh's ID
    virtual uint32_t UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices) = 0;
    virtual uint32_t UploadMesh(MeshAsset &asset) = 0;

    // Takes in a mesh ID and represents
    virtual void DestroyMesh(uint32_t meshID) = 0;

    // Establishes that the following commands apply to a new frame
    virtual bool InitFrame() = 0;

    // Sets the view of a camera
    virtual void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos) = 0;

    // Sets the mesh currently being rendered to
    virtual void SetMesh(uint32_t meshID) = 0;

    // Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
    virtual void SendObjectData(std::vector<ObjectData>& objects) = 0;

    // End the frame and present it to the screen
    virtual void EndFrame() = 0;

    // Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
    virtual void DrawObjects(int count, int startIndex) = 0;
};