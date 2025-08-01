#pragma once

#include <webgpu/webgpu.h>

#include "renderer/render_backend.h"
#include "renderer/wgpu_backend/bind_group_wgpu.h"
#include "renderer/wgpu_backend/utils_wgpu.h"
#include "renderer/wgpu_backend/render_types_wgpu.h"

#include "math/skl_math_consts.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <unordered_map>

// Allows for encapsulation of WebGPU render capabilities
class WGPURenderBackend {
private:
    u32 m_screenWidth{ 0 };
    u32 m_screenHeight{ 0 };

    // WGPU objects that remains important throughout rendering 
    // from init to destruction
    WGPUCore m_wgpuCore{ };
    WGPUQueue m_wgpuQueue{ };
    WGPUSurface m_wgpuSurface{ };

    // Stores best supported format on current device
    WGPUTextureFormat m_wgpuTextureFormat{ };
    WGPUTextureFormat m_wgpuDepthTextureFormat{ WGPUTextureFormat_Depth24Plus };

    // Represents limits of gpu storage
    u32 m_maxObjArraySize{ 4096 }; // TODO: Fill the following with number informed by limits
    u32 m_maxLightSpaces{ 4096 };
    u32 m_maxDynamicShadowedDirLights{ 4096 };
    u32 m_maxMeshVertSize{ 4096 };
    u32 m_maxMeshIndexSize{ 4096 };


    // Represents temporary variables that are inited/edited/and cleared over the course of frame
    WGPUSurfaceTexture m_surfaceTexture{ };
    WGPUTextureView m_surfaceTextureView{ }; 
    WGPUBackendTexture m_depthTexture{ };

    // Represents the current pass being drawn with
    WGPUCommandEncoder m_passCommandEncoder{ };
    WGPURenderPassEncoder m_renderPassEncoder{ };
    bool m_renderPassActive{ false };

    // Defines default pipeline
    WGPURenderPipeline m_defaultPipeline{ };
    WGPUBackendBindGroup m_bindGroup;

    // Defines depth pipeline
    WGPURenderPipeline m_depthPipeline{ };
    WGPUBackendBindGroup m_depthBindGroup;

    // Defines general light vars
    u32 m_nextLightSpace = 0;
    WGPUTexture m_shadowAtlas; // Stores depth textures to prevent constant recreation of such textures

    // Defines dir light vars, the key is the Shadow Id
    std::vector<WGPUBackendDynamicShadowedDirLightData> m_dynamicShadowedDirLights;

    WGPUBackendSingleUniformBuffer<WGPUBackendCameraData> m_cameraBuffer{ };
    WGPUBackendSingleUniformBuffer<glm::mat4x4> m_cameraSpaceBuffer{ };
    WGPUBackendSingleStorageArrayBuffer<WGPUBackendObjectData> m_instanceDatBuffer{ };
    WGPUBackendSingleStorageArrayBuffer<glm::mat4x4> m_lightSpacesStoreBuffer{ };
    WGPUBackendSingleStorageArrayBuffer<WGPUBackendDynamicShadowedDirLightData> m_dynamicShadowedDirLightBuffer{ };

    WGPUBackendArrayBuffer<Vertex> m_meshVertexBuffer{ };
    WGPUBackendArrayBuffer<u32> m_meshIndexBuffer{ };
    u32 m_meshTotalVertices{ 0 };
    u32 m_meshTotalIndices{ 0 };
    // Currently mesh deletion logic requires that meshes with greater MeshID's to correspond to older mesh stores
    std::unordered_map<MeshID, WGPUBackendMeshIdx> m_meshStore{ };

    // The id of the next obj that will be created
    MeshID m_nextMeshID{ 0 }; 

    void printDeviceSpecs();

    // The following getters occur asynchronously in wgpu but is awaited for by these functions
    static WGPUAdapter GetAdapter(const WGPUInstance instance, WGPURequestAdapterOptions const * options);

    static WGPUDevice GetDevice(const WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor);

    // What to call on the queue finishing its work
    static void QueueFinishCallback(WGPUQueueWorkDoneStatus status, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

    // What to call on m_wgpuDevice being lost.
    static void LostDeviceCallback(WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

    // What to call on WebGPU error
    static void ErrorCallback(WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

    // Fills in related directional light
    void PrepareDynamicShadowedDirLights(
        const glm::mat4x4& camView,
        const float camAspect,
        const float camFov, 
        const float camNear, 
        const float camFar, 
        const std::vector<DirLightRenderInfo>& gotDirLightRenderInfo);

    // Establishes that the following commands apply to a new frame
    bool InitFrame();

    // Begins the final color pass that renders frame to color pass
    void BeginColorPass();

    // Populates depth buffer from view of camera buffer
    void BeginDepthPass(WGPUTextureView depthTexture);

    // TODO: Actually implement
    void SetDirLight(LightCascade* cascades, glm::vec3 lightDir, TextureID texture) { };
    
    // Stops the current pass
    void EndPass();
    
    // Draws engine interface for game if allowed
    void DrawImGui();

    // Takes in mesh counts and renders to current command encoder using previously
    // inserted object data in buffer.
    void DrawObjects(std::map<u32, u32>& meshCounts);

    // Ends the current pass and present it to the screen
    void EndFrame();
public:
    // No logic needed
    WGPURenderBackend() { }

    ~WGPURenderBackend();

    // No SDL flags are needed with webgpu
    SDL_WindowFlags GetRenderWindowFlags() { return 0; }

    // Sets a SDL window to draw to and initializes the back end
    void InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight);

    // Sets up pipelines used to render
    void InitPipelines();

    // Renders and displays frame based on state
    void RenderUpdate(RenderFrameInfo& state);

    // Moves mesh to the GPU, 
    // Returns a uint that represents the mesh's ID
    MeshID UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices);

    // Removes mesh from GPU and render's mesh ID invalid
    void DestroyMesh(MeshID meshID);
};
