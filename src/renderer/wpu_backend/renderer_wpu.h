#pragma once

#include "renderer/render_types.h"
#include "math/math_consts.h"
#include "asset_types.h"

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>

// Allows for encapsulation of WebGPU render capabilities
class WPURenderBackend {
public:
    WPURenderBackend() { }

    // Gets the SDL Flags eneded
    SDL_WindowFlags GetRenderWindowFlags() { return 0; }

    // Sets a SDL window to draw to and initializes the back end
    void InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight);

    // Moves mesh to the GPU, 
    // Returns a uint that represents the mesh's ID
    uint32_t UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices) { return 0; };
    uint32_t UploadMesh(MeshAsset &asset) { return 0; }

    // Takes in a mesh ID and represents
    void DestroyMesh(uint32_t meshID) { }

    // Establishes that the following commands apply to a new frame
    bool InitFrame() { return false; }

    // Sets the view of a camera
    void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos) { }

    // Sets the mesh currently being rendered to
    void SetMesh(uint32_t meshID) { }

    // Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
    void SendObjectData(std::vector<ObjectData>& objects) { }

    // End the frame and present it to the screen
    void EndFrame() { }

    // Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
    void DrawObjects(int count, int startIndex) { }
};