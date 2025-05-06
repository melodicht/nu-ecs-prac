struct AllocatedBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
};

struct AllocatedImage
{
    VkImage image;
    VmaAllocation allocation;
};


// The handler for a mesh uploaded to the GPU
struct Mesh
{
    AllocatedBuffer indexBuffer;
    AllocatedBuffer vertBuffer;
    VkDeviceAddress vertAddress;

    u32 indexCount;
};

struct CameraData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 pos;
};

struct ObjectData
{
    glm::mat4 model;
    glm::vec4 color;
};

struct PushConstants
{
    VkDeviceAddress cameraAddress;
    VkDeviceAddress objectAddress;
    VkDeviceAddress vertexAddress;
};

// The data that is stored per frame in flight
struct FrameData
{
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    AllocatedBuffer cameraBuffer;
    VkDeviceAddress cameraAddress;
    AllocatedBuffer objectBuffer;
    VkDeviceAddress objectAddress;
};