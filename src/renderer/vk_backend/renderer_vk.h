#pragma once

#include "vk_render_utils.cpp"
#include "render_backend.h"

#include <map>

class VKRenderBackend : public IRenderBackend {
private:
// Vulkan structures
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;

    VmaAllocator allocator;

    VkFormat depthFormat;
    AllocatedImage depthImage;
    VkImageView depthImageView;

    VkFormat swapchainFormat;
    VkSwapchainKHR swapchain;
    VkExtent2D swapExtent;
    std::vector<VkImage> swapImages;
    std::vector<VkImageView> swapImageViews;

    std::vector<VkSemaphore> renderSemaphores;

    #define NUM_FRAMES 2

    VkCommandPool mainCommandPool;
    FrameData frames[NUM_FRAMES];

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    uint32_t frameNum{ 0 };

    uint32_t swapIndex{ 0 };
    bool resize = false;
    uint32_t indexCount{ 0 };

    uint32_t currentMeshID{ 0 };
    std::map<uint32_t,Mesh> meshStore;

    // Create swapchain or recreate to change size
    void CreateSwapchain(uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain);
    void DestroySwapResources();
    void RecreateSwapchain();
public:
    // No real logic is needed for this
    VKRenderBackend() {}

    // Initialize the rendering API
    virtual void InitRenderer(SDL_Window *window) override;

    // Upload a mesh to the gpu
    virtual uint32_t UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices) override;
    virtual uint32_t UploadMesh(MeshAsset &asset) override;

    virtual void DestroyMesh(uint32_t meshID) override;

    // Set up frame and begin capturing draw calls
    virtual bool InitFrame() override;

    // Set the matrices of the camera (Must be called between InitFrame and EndFrame)
    virtual void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos) override;

    // Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
    virtual void SendObjectData(std::vector<ObjectData>& objects) override;

    // Set the mesh currently being rendered (Must be called between InitFrame and EndFrame)
    virtual void SetMesh(Mesh* mesh) override;

    // Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
    virtual void DrawObjects(int count, int startIndex) override;

    // End the frame and present it to the screen
    virtual void EndFrame() override;
};