#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			printf("Detected Vulkan error: %d\n", err);             \
			abort();                                                \
		}                                                           \
	} while (0)

#include "vk_render_types.h"
#include "vk_render_utils.cpp"


// Vulkan structures
VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;
VkSurfaceKHR surface;

VkPhysicalDevice physDevice;
VkDevice device;

VkQueue graphicsQueue;
u32 graphicsQueueFamily;

VmaAllocator allocator;

VkFormat depthFormat;
AllocatedImage depthImage;
VkImageView depthImageView;

VkFormat swapchainFormat;
VkSwapchainKHR swapchain;
VkExtent2D swapExtent;
std::vector<VkImage> swapImages;
std::vector<VkImageView> swapImageViews;

#define NUM_FRAMES 2

VkCommandPool mainCommandPool;
FrameData frames[NUM_FRAMES];

VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

u32 frameNum;


// Upload a mesh to the gpu
Mesh* UploadMesh(u32 vertCount, glm::vec4* vertices, u32 indexCount, u32* indices)
{
    Mesh* mesh = new Mesh();

    size_t indexSize = sizeof(u32) * indexCount;
    size_t vertSize = sizeof(glm::vec4) * vertCount;

    mesh->indexBuffer = CreateBuffer(allocator,
                                     indexSize,
                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                                     | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                     0,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mesh->vertBuffer = CreateBuffer(allocator, vertSize,
                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                                    | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                                    | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                    0,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkBufferDeviceAddressInfo deviceAddressInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = mesh->vertBuffer.buffer};
    mesh->vertAddress = vkGetBufferDeviceAddress(device, &deviceAddressInfo);


    AllocatedBuffer stagingBuffer = CreateBuffer(allocator,
                                                 indexSize + vertSize,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VMA_ALLOCATION_CREATE_MAPPED_BIT
                                                 | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void* stagingData = stagingBuffer.allocation->GetMappedData();
    memcpy(stagingData, indices, indexSize);
    memcpy((char*)stagingData + indexSize, vertices, vertSize);

    VkCommandBuffer cmd = BeginImmediateCommands(device, mainCommandPool);

    VkBufferCopy indexCopy{0};
    indexCopy.dstOffset = 0;
    indexCopy.srcOffset = 0;
    indexCopy.size = indexSize;

    vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh->indexBuffer.buffer, 1, &indexCopy);

    VkBufferCopy vertCopy{0};
    vertCopy.dstOffset = 0;
    vertCopy.srcOffset = indexSize;
    vertCopy.size = vertSize;

    vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh->vertBuffer.buffer, 1, &vertCopy);

    EndImmediateCommands(device, graphicsQueue, mainCommandPool, cmd);
    DestroyBuffer(allocator, stagingBuffer);


    mesh->indexCount = indexCount;

    return mesh;
}

void DestroyMesh(Mesh* mesh)
{
    DestroyBuffer(allocator, mesh->indexBuffer);
    DestroyBuffer(allocator, mesh->vertBuffer);
}


// Create swapchain or recreate to change size
void CreateSwapchain(u32 width, u32 height, VkSwapchainKHR oldSwapchain)
{
    // Create the swapchain
    vkb::SwapchainBuilder swapBuilder{physDevice, device, surface};

    swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapBuilder
            .set_desired_format(VkSurfaceFormatKHR{.format = swapchainFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .set_old_swapchain(oldSwapchain)
            .build().value();

    swapchain = vkbSwapchain.swapchain;
    swapExtent = vkbSwapchain.extent;
    swapImages = vkbSwapchain.get_images().value();
    swapImageViews = vkbSwapchain.get_image_views().value();


    // Create the depth buffer
    depthFormat = VK_FORMAT_D32_SFLOAT;

    VkExtent3D depthImageExtent =
            {
                    swapExtent.width,
                    swapExtent.height,
                    1
            };

    depthImage = CreateImage(allocator,
                             depthFormat,
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                             depthImageExtent,
                             VMA_MEMORY_USAGE_GPU_ONLY,
                             VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

    VkImageViewCreateInfo depthViewInfo{};
    depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthViewInfo.format = depthFormat;
    depthViewInfo.image = depthImage.image;
    depthViewInfo.subresourceRange.baseMipLevel = 0;
    depthViewInfo.subresourceRange.levelCount = 1;
    depthViewInfo.subresourceRange.baseArrayLayer = 0;
    depthViewInfo.subresourceRange.layerCount = 1;
    depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    VK_CHECK(vkCreateImageView(device, &depthViewInfo, nullptr, &depthImageView));
}

void DestroySwapResources()
{
    vkDestroyImageView(device, depthImageView, nullptr);
    DestroyImage(allocator, depthImage);

    for (VkImageView imageView : swapImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
}

void RecreateSwapchain()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &surfaceCapabilities);

    u32 width = surfaceCapabilities.currentExtent.width;
    u32 height = surfaceCapabilities.currentExtent.height;

    if (width == 0 || height == 0)
    {
        return;
    }

    vkDeviceWaitIdle(device);

    DestroySwapResources();

    VkSwapchainKHR old = swapchain;
    CreateSwapchain(width, height, old);
    vkDestroySwapchainKHR(device, old, nullptr);
}


// Initialize the rendering API
void InitRenderer(SDL_Window *window)
{
    // Create Vulkan instance
    vkb::InstanceBuilder builder;
    vkb::Instance vkbInstance = builder
            .set_app_name("Untitled Engine")
            .request_validation_layers()
            .use_default_debug_messenger()
            .require_api_version(1, 3)
            .build().value();


    instance = vkbInstance.instance;
    volkLoadInstance(instance);
    debugMessenger = vkbInstance.debug_messenger;


    // Create window surface
    SDL_Vulkan_CreateSurface(window, instance, vkbInstance.allocation_callbacks, &surface);

    VkPhysicalDeviceVulkan11Features feat11{};
    feat11.shaderDrawParameters = true;

    VkPhysicalDeviceVulkan12Features feat12{};
    feat12.bufferDeviceAddress = true;

    VkPhysicalDeviceVulkan13Features feat13{};
    feat13.dynamicRendering = true;
    feat13.synchronization2 = true;


    // Select which GPU to use
    vkb::PhysicalDeviceSelector selector{vkbInstance};
    vkb::PhysicalDevice vkbPhysDevice = selector
            .set_surface(surface)
            .set_minimum_version(1, 3)
            .set_required_features_11(feat11)
            .set_required_features_12(feat12)
            .set_required_features_13(feat13)
            .prefer_gpu_device_type()
            .select().value();
    physDevice = vkbPhysDevice.physical_device;


    // Create the logical GPU device
    vkb::DeviceBuilder deviceBuilder{vkbPhysDevice};
    vkb::Device vkbDevice = deviceBuilder.build().value();
    device = vkbDevice.device;
    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    volkLoadDevice(device);


    // Create the VMA allocator
    VmaVulkanFunctions vmaFuncs{};
    vmaFuncs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaFuncs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocInfo = {};
    allocInfo.physicalDevice = physDevice;
    allocInfo.device = device;
    allocInfo.instance = instance;
    allocInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocInfo.pVulkanFunctions = &vmaFuncs;

    VK_CHECK(vmaCreateAllocator(&allocInfo, &allocator));

    // Create the swapchain and associated resources at the default dimensions
    CreateSwapchain(WINDOW_WIDTH, WINDOW_HEIGHT, nullptr);

    // Create the command pools and command buffers
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.queueFamilyIndex = graphicsQueueFamily;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &mainCommandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    for (int i = 0; i < NUM_FRAMES; i++)
    {
        VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frames[i].commandPool));

        cmdAllocInfo.commandPool = frames[i].commandPool;
        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frames[i].commandBuffer));
    }

    // Create the sync structures
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    for (int i = 0; i < NUM_FRAMES; i++)
    {
        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &frames[i].renderFence));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frames[i].renderSemaphore));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frames[i].presentSemaphore));
    }



    // Create camera and object buffers
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        frames[i].cameraBuffer = CreateBuffer(allocator,
                                              sizeof(CameraData),
                                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                                              | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                              VMA_ALLOCATION_CREATE_MAPPED_BIT
                                              | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        VkBufferDeviceAddressInfo cameraAddressInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                                                    .buffer = frames[i].cameraBuffer.buffer};
        frames[i].cameraAddress = vkGetBufferDeviceAddress(device, &cameraAddressInfo);



        frames[i].objectBuffer = CreateBuffer(allocator,
                                              sizeof(glm::mat4) * 4096,
                                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                                              | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                              VMA_ALLOCATION_CREATE_MAPPED_BIT
                                              | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        VkBufferDeviceAddressInfo objectAddressInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                                                    .buffer = frames[i].objectBuffer.buffer};
        frames[i].objectAddress = vkGetBufferDeviceAddress(device, &objectAddressInfo);
    }

    // Create shader stages
    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    void *shaderFile = SDL_LoadFile("shaders/shader.spv", &shaderInfo.codeSize);
    shaderInfo.pCode = reinterpret_cast<const uint32_t*>(shaderFile);
    VkShaderModule shader;
    VK_CHECK(vkCreateShaderModule(device, &shaderInfo, nullptr, &shader));

    VkPipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = shader;
    vertStageInfo.pName = "vertexMain";

    VkPipelineShaderStageCreateInfo fragStageInfo{};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = shader;
    fragStageInfo.pName = "fragmentMain";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};


    // Create render pipeline layout
    VkPushConstantRange pushConstants;
    pushConstants.offset = 0;
    pushConstants.size = sizeof(PushConstants);
    pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstants;

    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));


    // Create render pipeline (AKA fill in 20000 info structs)
    std::vector<VkDynamicState> dynamicStates =
    {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInfo{};  // We are using vertex pulling so no need for vertex input description
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkPipelineRenderingCreateInfo dynRenderInfo{};
    dynRenderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    dynRenderInfo.colorAttachmentCount = 1;
    dynRenderInfo.pColorAttachmentFormats = &swapchainFormat;
    dynRenderInfo.depthAttachmentFormat = depthFormat;
    dynRenderInfo.depthAttachmentFormat = depthFormat;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = nullptr; // No renderpass necessary because we are using dynamic rendering
    pipelineInfo.subpass = 0;
    pipelineInfo.pNext = &dynRenderInfo;

    VK_CHECK(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline));

    vkDestroyShaderModule(device, shader, nullptr);
}

uint32_t swapIndex;

bool resize = false;

// Set up frame and begin capturing draw calls
bool InitFrame()
{
    //Set up commands
    VkCommandBuffer& cmd = frames[frameNum].commandBuffer;

    //Synchronize and get images
    VK_CHECK(vkWaitForFences(device, 1, &frames[frameNum].renderFence, true, 1000000000));

    VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, 1000000000, frames[frameNum].presentSemaphore, nullptr, &swapIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return false;
    }
    if (acquireResult == VK_SUBOPTIMAL_KHR)
    {
        resize = true;
    }

    VK_CHECK(vkResetFences(device, 1, &frames[frameNum].renderFence));

    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    //Begin commands and rendering
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    TransitionImage(cmd, swapImages[swapIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkClearValue clearValue;
    clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = swapImageViews[swapIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = clearValue;

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = depthImageView;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil.depth = 1.0f;

    VkRenderingInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea.offset = {0, 0};
    renderInfo.renderArea.extent = swapExtent;
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachment;
    renderInfo.pDepthAttachment = &depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    //set dynamic viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = swapExtent.height;;
    viewport.width = (float)swapExtent.width;
    viewport.height = -(float)swapExtent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = swapExtent.width;
    scissor.extent.height = swapExtent.height;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return true;
}

// Set the matrices of the camera (Must be called between InitFrame and EndFrame)
void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos)
{
    CameraData camera{view, proj, pos};
    void* cameraData = frames[frameNum].cameraBuffer.allocation->GetMappedData();
    memcpy(cameraData, &camera, sizeof(CameraData));
}

// Send the matrices of the models to render (Must be called between InitFrame and EndFrame)
void SendModelMatrices(std::vector<glm::mat4>& matrices)
{
    void* objectData = frames[frameNum].objectBuffer.allocation->GetMappedData();
    memcpy(objectData, matrices.data(), sizeof(glm::mat4) * matrices.size());
}

u32 indexCount;

// Set the mesh currently being rendered (Must be called between InitFrame and EndFrame)
void SetMesh(Mesh* mesh)
{
    // Send addresses to camera, object, and vertex buffers as push constants
    VkCommandBuffer& cmd = frames[frameNum].commandBuffer;
    PushConstants pushConstants = {frames[frameNum].cameraAddress, frames[frameNum].objectAddress, mesh->vertAddress};
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);
    // Bind the index buffer
    vkCmdBindIndexBuffer(cmd, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    indexCount = mesh->indexCount;
}

// Draw multiple objects to the screen (Must be called between InitFrame and EndFrame and after SetMesh)
void DrawObjects(int count, int startIndex)
{
    vkCmdDrawIndexed(frames[frameNum].commandBuffer, indexCount, count, 0, 0, startIndex);
}

// End the frame and present it to the screen
void EndFrame()
{
    // End dynamic rendering and commands
    VkCommandBuffer& cmd = frames[frameNum].commandBuffer;

    vkCmdEndRendering(cmd);
    TransitionImage(cmd, swapImages[swapIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    //Submit commands
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &frames[frameNum].presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &frames[frameNum].renderSemaphore;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, frames[frameNum].renderFence));

    //Draw to screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &frames[frameNum].renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapIndex;

    VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || resize)
    {
        resize = false;
        RecreateSwapchain();
    }

    frameNum++;
    frameNum %= NUM_FRAMES;
}
