#pragma once

#include <webgpu/webgpu.h>

#include "renderer/render_types.h"
#include "renderer/wgpu_backend/render_types_wgpu.h"
#include "math/math_consts.h"
#include "asset_types.h"

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>

#include <unordered_map>

// Allows for encapsulation of WebGPU render capabilities
class WGPURenderBackend {
private:
    u32 m_screenWidth{ 0 };
    u32 m_screenHeight{ 0 };

    // WGPU objects that remains important throughout rendering 
    // from init to destruction
    WGPUInstance m_wgpuInstance{ };
    WGPUDevice m_wgpuDevice{ };
    WGPUQueue m_wgpuQueue{ };
    WGPUSurface m_wgpuSurface{ };
    WGPURenderPipeline m_wgpuPipeline{ };

    // Stores best supported format on current device
    WGPUTextureFormat m_wgpuTextureFormat{ };
    WGPUTextureFormat m_wgpuDepthTextureFormat{ WGPUTextureFormat_Depth24Plus };

    // Represents amount of objects that can be represented by a single mesh
    u32 m_maxObjArraySize{ 4096 }; // TODO: Fill with number informed by limits

    bool m_doingColorPass{ false }; // Currently does nothing happens whatsoever if it isn't a color pass
    // Depth stuff will come later

    // Represents temporary variables that are inited/edited/and cleared over the course of frame
    WGPUTextureView m_surfaceTextureView{ };
    WGPUSurfaceTexture m_surfaceTexture{ };
    WGPUTextureView m_depthTextureView{ };
    WGPUTexture m_depthTexture{ };
    WGPUTextureView m_depthTextureFormat{ };
    WGPUCommandEncoder m_meshCommandEncoder{ };
    WGPURenderPassEncoder m_meshPassEncoder{ };
    bool m_meshBufferActive{ }; // Determines whether current mesh commands needs to end for another to continue

    // Defines part of default pipeline
    WGPUBindGroup m_bindGroup{ };
    WGPUBuffer m_cameraBuffer{ };
    WGPUBuffer m_baseIndexBuffer{ };
    WGPUBuffer m_instanceIndexBuffer{ };
    WGPUBuffer m_storageBuffer{ };

    std::unordered_map<MeshID, Mesh> m_meshStore{ };
    std::unordered_map<CameraID, CameraData> m_cameraStore{ };
    MeshID m_nextMeshID{ 0 };        // The mesh ID of the next mesh that will be created
    CameraID m_nextCameraID{ 0 };    // The camera ID of the next camera that will be created
    MeshID m_currentMeshID{ 0 };     // The mesh currently being drawn in frame loop
    CameraID m_currentCameraID{ 0 }; // The camera currently being viewed from

    void printDeviceSpecs();

    // Translates a c_string to a wgpu string view
    static WGPUStringView wgpuStr(const char* str);

    // Creates a default rendering pipeline
    void CreateDefaultPipeline();

    // Ends current mesh pass if exists
    void EndMeshPass();

    // The following getters occur asynchronously in wgpu but is awaited for by these functions
    static WGPUAdapter GetAdapter(const WGPUInstance instance, WGPURequestAdapterOptions const * options);

    static WGPUDevice GetDevice(const WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor);

    // What to call on the a queue finishing its work
    static void QueueFinishCallback(WGPUQueueWorkDoneStatus status, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

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
    MeshID UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices);
    MeshID UploadMesh(MeshAsset &asset);

    // Designates a camera as part of the render pass 
    CameraID AddCamera();

    TextureID CreateDepthTexture(u32 width, u32 height) { return 0; }
    
    void DestroyTexture(TextureID textureID) { };

    // Takes in a mesh ID and represents
    void DestroyMesh(MeshID meshID) { }

    // Establishes that the following commands apply to a new frame
    bool InitFrame();

    void SetCamera(CameraID camera);

    void SetDirLight(glm::mat4 lightSpace, glm::vec3 lightDir, TextureID texture){ };

    // Sets the view of a camera
    void UpdateCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos);

    void BeginDepthPass(CullMode cullMode) { }

    void BeginDepthPass(TextureID target, CullMode cullMode) { }

    void BeginColorPass(CullMode cullMode);
    
    void EndPass();
    
    void DrawImGui() { }
    
    // Sets the mesh currently being rendered to
    void SetMesh(MeshID meshID);

    // Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
    void SendObjectData(std::vector<ObjectData>& objects);

    // End the frame and present it to the screen
    void EndFrame();

    // Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
    void DrawObjects(int count, int startIndex);
};
