// Represents a data buffer stored on the GPU
struct AllocatedBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VkDeviceAddress address;
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

    u32 indexCount;
};

// Represents a texture stored on the GPU
struct Texture
{
    AllocatedImage texture;
    VkImageView imageView;
    VkSampler sampler;
    VkExtent2D extent;
    u32 descriptorIndex;
};

// Represents the GPU memory locations of the camera, object, and vertex buffers (CPU->GPU)
struct VertPushConstants
{
    VkDeviceAddress objectAddress;
    VkDeviceAddress vertexAddress;
};

// Represents the direction of the skylight, and the descriptor id of the shadowmap (CPU->GPU)
struct FragPushConstants
{
    glm::vec3 lightDir;
    u32 shadowID;
    VkDeviceAddress lightAddress;
    u32 cascadeCount;
};

// Represents the data for a single frame in flight of rendering
struct FrameData
{
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore acquireSemaphore;
    VkFence renderFence;

    std::vector<AllocatedBuffer> cameraBuffers;
    AllocatedBuffer objectBuffer;
    AllocatedBuffer lightBuffer;
};