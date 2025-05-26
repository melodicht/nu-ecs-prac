#pragma once

#include <webgpu/webgpu.h>

#include "renderer/render_types.h"
#include "math/math_consts.h"
#include "asset_types.h"

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>

// Allows for encapsulation of WebGPU render capabilities
class WGPURenderBackend {
private:
    // WGPU objects important throughout entire lifetime
    WGPUInstance m_wgpuInstance{ };
    WGPUDevice m_wgpuDevice{ };
    WGPUQueue m_wgpuQueue{ };
    WGPUSurface m_wgpuSurface{ };

    // WGPU objects that may 

    void printDeviceSpecs();

    // Translates a c_string to a wgpu string view
    static WGPUStringView wgpuStr(const char* str);

    // The following getters occur asynchronously in wgpu but is awaited for by these functions
    static WGPUAdapter GetAdapter(const WGPUInstance instance, WGPURequestAdapterOptions const * options);

    static WGPUDevice GetDevice(const WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor);

    // What to call on the a queue finishing its work
    static void QueueFinishCallback(WGPUQueueWorkDoneStatus status, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

    // What to call on m_wgpuDevice being lost.
    static void LostDeviceCallback(WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

    // What to call on WebGPU error
    static void ErrorCallback(WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

public:
    // No logic needed
    WGPURenderBackend() { }

    ~WGPURenderBackend();

    // Gets the SDL Flags eneded
    SDL_WindowFlags GetRenderWindowFlags() { return 0; }

    // Sets a SDL window to draw to and initializes the back end
    void InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight);

    // Moves mesh to the GPU, 
    // Returns a uint that represents the mesh's ID
    uint32_t UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices) { return 0; };
    uint32_t UploadMesh(MeshAsset &asset) { return 0; }

    // Designates a camera as part of the render pass 
    CameraID AddCamera() { return 0; }

    TextureID CreateDepthTexture(u32 width, u32 height) { return 0; }
    
    void DestroyTexture(TextureID textureID) { };

    // Takes in a mesh ID and represents
    void DestroyMesh(uint32_t meshID) { }

    // Establishes that the following commands apply to a new frame
    bool InitFrame();

    void SetCamera(CameraID camera) { }

    void SetDirLight(glm::mat4 lightSpace, glm::vec3 lightDir, TextureID texture) { }

    // Sets the view of a camera
    void UpdateCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos) { }

    void BeginDepthPass(CullMode cullMode) { }

    void BeginDepthPass(TextureID target, CullMode cullMode) { }

    void BeginColorPass(CullMode cullMode) { }
    
    void EndPass() { }
    
    void DrawImGui() { }
    
    // Sets the mesh currently being rendered to
    void SetMesh(uint32_t meshID) { }

    // Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
    void SendObjectData(std::vector<ObjectData>& objects) { }

    // End the frame and present it to the screen
    void EndFrame() { }

    // Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
    void DrawObjects(int count, int startIndex) { }
};