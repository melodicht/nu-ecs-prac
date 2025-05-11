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

// Represents a vertex of a mesh (CPU->GPU)
struct Vertex
{
    glm::vec3 position;
    f32 uvX;
    glm::vec3 normal;
    f32 uvY;
};

// Represents the transformation data of the camera (CPU->GPU)
struct CameraData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 pos;
};

// Represents the transformation data of the objects in the scene (CPU->GPU)
struct ObjectData
{
    glm::mat4 model;
    glm::vec4 color;
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