// Represents a data buffer stored on the GPU
struct AllocatedBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
};

// Represents an image stored on the GPU
struct AllocatedImage
{
    VkImage image;
    VmaAllocation allocation;
};


// Represents a mesh stored on the GPU
struct Mesh
{
    AllocatedBuffer indexBuffer;
    AllocatedBuffer vertBuffer;
    VkDeviceAddress vertAddress;

    u32 indexCount;
};

// Represents a texture stored on the GPU
struct Texture
{
    AllocatedImage texture;
    VkImageView imageView;
    VkSampler sampler;
};

// Represents the GPU memory locations of the camera, object, and vertex buffers (CPU->GPU)
struct PushConstants
{
    VkDeviceAddress cameraAddress;
    VkDeviceAddress objectAddress;
    VkDeviceAddress vertexAddress;
};

// Represents the data for a single frame in flight of rendering
struct FrameData
{
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore acquireSemaphore;
    VkFence renderFence;

    AllocatedBuffer cameraBuffer;
    VkDeviceAddress cameraAddress;
    AllocatedBuffer objectBuffer;
    VkDeviceAddress objectAddress;
};