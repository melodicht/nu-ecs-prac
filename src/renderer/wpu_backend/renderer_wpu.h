#pragma once

#include "render_backend.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Currently is a bit of a dummy class 
class WPURenderBackend : public IRenderBackend {
public:
    WPURenderBackend() {}

    // Sets a SDL window to draw to and initializes the back end
    virtual void InitRenderer(SDL_Window *window) override;

    // Moves mesh to the GPU, 
    // Returns a uint that represents the mesh's ID
    virtual uint32_t UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices) override { return 0; };
    virtual uint32_t UploadMesh(MeshAsset &asset) override { return 0; };

    // Takes in a mesh ID and represents
    virtual void DestroyMesh(uint32_t meshID) override { };

    // Establishes that the following commands apply to a new frame
    virtual bool InitFrame() override { return false; };

    // Sets the view of a camera
    virtual void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos) override { };

    // Sets the mesh currently being rendered to
    virtual void SetMesh(uint32_t meshID) override { };

    // Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
    virtual void SendObjectData(std::vector<ObjectData>& objects) override { };

    // End the frame and present it to the screen
    virtual void EndFrame() override { };

    // Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
    virtual void DrawObjects(int count, int startIndex) override { };
};