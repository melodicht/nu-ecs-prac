#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>

#if SKL_ENABLED_EDITOR
#include <backends/imgui_impl_wgpu.h>
#endif

#include "meta_definitions.h"

#include "renderer/wgpu_backend/renderer_wgpu.h"
#include "webgpu/sdl3webgpu-main/sdl3webgpu.h"

// Much of this was taken from https://eliemichel.github.io/LearnWebGPU

#pragma region Helper Functions
void WGPURenderBackend::printDeviceSpecs() {
  WGPUSupportedFeatures features;
  wgpuDeviceGetFeatures(m_wgpuDevice, &features);

  LOG("Device features:");
  for (int iter = 0; iter < features.featureCount ; iter++) {
      LOG(" - 0x" << features.features[iter]);
  }

  WGPULimits limits = {};
  limits.nextInChain = nullptr;

  WGPUStatus success = wgpuDeviceGetLimits(m_wgpuDevice, &limits);

  if (success == WGPUStatus_Success) {
      LOG("Device limits:");
      LOG(" - maxTextureDimension1D: " << limits.maxTextureDimension1D);
      LOG(" - maxTextureDimension2D: " << limits.maxTextureDimension2D);
      LOG(" - maxTextureDimension3D: " << limits.maxTextureDimension3D);
      LOG(" - maxTextureArrayLayers: " << limits.maxTextureArrayLayers);
      // [...] Extra device limits
  }
}

WGPUStringView WGPURenderBackend::wgpuStr(const char* string) {
  WGPUStringView retString {
    .data = string,
    .length = std::strlen(string)
  };
  return retString;
}

WGPUBindGroupLayoutEntry DefaultBindLayoutEntry() {
  return WGPUBindGroupLayoutEntry {
    .nextInChain = nullptr,
    .binding = 0,
    .visibility = WGPUShaderStage_None,
    .buffer {
      .nextInChain = nullptr,
      .type = WGPUBufferBindingType_Undefined,
      .hasDynamicOffset = false,
      .minBindingSize = 0,
    },
    .sampler {
      .nextInChain = nullptr,
      .type = WGPUSamplerBindingType_BindingNotUsed,
    },
    .texture {
      .nextInChain = nullptr,
      .sampleType = WGPUTextureSampleType_BindingNotUsed,
      .viewDimension =  WGPUTextureViewDimension_Undefined,
      .multisampled = false,
    },
    .storageTexture {
      .nextInChain = nullptr,
      .access = WGPUStorageTextureAccess_BindingNotUsed,
      .format = WGPUTextureFormat_Undefined,
      .viewDimension = WGPUTextureViewDimension_Undefined,
    },
  };
}

void WGPURenderBackend::EndMeshPass() {
  if(m_meshBufferActive) {
    wgpuRenderPassEncoderEnd(m_meshPassEncoder);
    wgpuRenderPassEncoderRelease(m_meshPassEncoder);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {
      .nextInChain = nullptr,
      .label =  wgpuStr("Mesh Command Buffer"),
    };
  
    WGPUCommandBuffer meshCommand = wgpuCommandEncoderFinish(m_meshCommandEncoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(m_meshCommandEncoder);
  
    wgpuQueueSubmit(m_wgpuQueue, 1, &meshCommand);
    wgpuCommandBufferRelease(meshCommand);
    m_meshBufferActive = false;
  }
}

WGPUAdapter WGPURenderBackend::GetAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options) {
  WGPUAdapter set = nullptr;
  bool requestEnded = false;

  WGPURequestAdapterCallbackInfo callbackInfo;
  callbackInfo.callback = [](WGPURequestAdapterStatus status, WGPUAdapterImpl* adapter, WGPUStringView message, void *userdata1, void *userdata2) 
  {
      if (status == WGPURequestAdapterStatus_Success) {
        *((WGPUAdapter *)userdata1) = adapter;
      } else {
          // LOG("Could not get WebGPU adapter: " << (message.data));
          LOG("Could not get WebGPU adapter: " << (message.data));
      }
      *(bool *)userdata2 = true;
  };

  callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  callbackInfo.userdata1 = &set;
  callbackInfo.userdata2 = &requestEnded;

  // Call to the WebGPU request adapter procedure
  wgpuInstanceRequestAdapter(
      instance /* equivalent of navigator.gpu */,
      options,
      callbackInfo
  );
  // We wait until userData.requestEnded gets true
  #if EMSCRIPTEN
  while (!requestEnded) {
      emscripten_sleep(100);
  }
  #endif

  assert(requestEnded);

  return set;
}

WGPUDevice WGPURenderBackend::GetDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
  WGPUDevice set = nullptr;
  bool requestEnded = false;

  WGPURequestDeviceCallbackInfo callbackInfo;
  callbackInfo.callback = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void * userdata1, void * userdata2) {
      if (status == WGPURequestDeviceStatus_Success) {
        *((WGPUDevice *)userdata1) = device;
      } else {
          LOG("Could not get WebGPU device: " << message.data);
      }
      *((bool *)userdata2) = true;
  };
  callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  callbackInfo.userdata1 = &set;
  callbackInfo.userdata2 = &requestEnded;

  wgpuAdapterRequestDevice(
      adapter,
      descriptor,
      callbackInfo
  );

#ifdef __EMSCRIPTEN__
  while (!requestEnded) {
      emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__

  assert(requestEnded);

  return set;
}

void WGPURenderBackend::QueueFinishCallback(WGPUQueueWorkDoneStatus status, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
  LOG("Queued work finished with status: " << status);
  LOG("Included Message: " << message.data);
}

void WGPURenderBackend::LostDeviceCallback(WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
  LOG("Device lost: reason " << reason);
  if (message.data) LOG(" (" << message.data << ")");
}

void WGPURenderBackend::ErrorCallback(WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
  LOG("Error happened: error " << type);
  if (message.data) LOG("(" << message.data << ")");
}
#pragma endregion

#pragma region Interface Impl
WGPURenderBackend::~WGPURenderBackend() {
  wgpuSurfaceUnconfigure(m_wgpuSurface);
  wgpuSurfaceRelease(m_wgpuSurface);
  wgpuQueueRelease(m_wgpuQueue);
  wgpuDeviceRelease(m_wgpuDevice);
  wgpuInstanceRelease(m_wgpuInstance);
}

void WGPURenderBackend::InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight) {
  m_screenHeight = startHeight;
  m_screenWidth = startWidth;

  // Creates instance
  WGPUInstanceDescriptor instanceDescriptor { 
    .nextInChain = nullptr
  };

  #if EMSCRIPTEN
  m_wgpuInstance = wgpuCreateInstance(nullptr);
  #else
  m_wgpuInstance = wgpuCreateInstance(&instanceDescriptor);
  #endif
  if (m_wgpuInstance == nullptr) {
    LOG_ERROR("Instance creation failed!");
    return;
  }
  LOG("Instance Created" << m_wgpuInstance);

  LOG("Requesting adapter...");

  m_wgpuSurface = SDL_GetWGPUSurface(m_wgpuInstance, window);
  WGPURequestAdapterOptions adapterOpts = {
    .nextInChain = nullptr,
    .compatibleSurface = m_wgpuSurface
  };

  WGPUAdapter adapter = GetAdapter(m_wgpuInstance, &adapterOpts);

  LOG("Got adapter: " << adapter);

  LOG("Requesting device...");

  // General device description
  WGPUDeviceDescriptor deviceDesc = {
    .nextInChain = nullptr,
    .label = wgpuStr("My Device"),
    .requiredFeatureCount = 0,
    .requiredLimits = nullptr,
    .defaultQueue {
      .nextInChain = nullptr,
      .label = wgpuStr("Default Queue")
    },
    .deviceLostCallbackInfo {
      .mode = WGPUCallbackMode_AllowSpontaneous,
      .callback = LostDeviceCallback
    },
    .uncapturedErrorCallbackInfo {
      .nextInChain = nullptr,
      .callback = ErrorCallback
    }
  };

  m_wgpuDevice = GetDevice(adapter, &deviceDesc);

  LOG("Got device: " << m_wgpuDevice);

  printDeviceSpecs();

  m_wgpuQueue = wgpuDeviceGetQueue(m_wgpuDevice);

  WGPUQueueWorkDoneCallbackInfo queueDoneCallback =  WGPUQueueWorkDoneCallbackInfo {
    .mode = WGPUCallbackMode_AllowProcessEvents,
    #if EMSCRIPTEN // TODO: It seems that emdawn has split off from native for now, check frequently
    .callback = nullptr,
    #else
    .callback = QueueFinishCallback,
    #endif
  };

  wgpuQueueOnSubmittedWorkDone(m_wgpuQueue, queueDoneCallback);

  WGPUSurfaceCapabilities capabilities { };
  wgpuSurfaceGetCapabilities(m_wgpuSurface, adapter, &capabilities );
  m_wgpuTextureFormat = capabilities.formats[0];
  WGPUSurfaceConfiguration config { 
    .nextInChain = nullptr,
    .device = m_wgpuDevice,
    .format = m_wgpuTextureFormat,
    .usage = WGPUTextureUsage_RenderAttachment,
    .width = startWidth,
    .height = startHeight,
    .viewFormatCount = 0,
    .viewFormats = nullptr,
    .alphaMode = WGPUCompositeAlphaMode_Auto,
    .presentMode = WGPUPresentMode_Fifo
  };

  wgpuSurfaceCapabilitiesFreeMembers( capabilities );

  wgpuSurfaceConfigure(m_wgpuSurface, &config);
  wgpuAdapterRelease(adapter);

  // Initializes imgui
  #if SKL_ENABLED_EDITOR
  ImGui_ImplWGPU_InitInfo imguiInit;
  imguiInit.Device = m_wgpuDevice;
  imguiInit.RenderTargetFormat = m_wgpuTextureFormat;
  imguiInit.DepthStencilFormat = m_wgpuDepthTextureFormat;
  imguiInit.NumFramesInFlight = 3;

  ImGui_ImplWGPU_Init(&imguiInit);

  ImGui_ImplWGPU_NewFrame();
  #endif
}

void WGPURenderBackend::InitPipelines(u32 numCascades)
{
  // Loads in shader module
  size_t loadedDatSize;
  auto loadedDat = SDL_LoadFile("shaders/default_shader.wgsl", &loadedDatSize);

  // Makes sure data actually gets loaded in
  assert(loadedDat);

  // SDL_free(loadedDat);

  WGPUShaderModuleWGSLDescriptor wgslShaderDesc {
    .chain {
      .next = nullptr,
      .sType = WGPUSType_ShaderSourceWGSL,
    },
    .code{
      .data = reinterpret_cast<const char *>(loadedDat),
      .length = loadedDatSize,
    },
  };

  WGPUShaderModuleDescriptor shaderDesc {
    .nextInChain = &wgslShaderDesc.chain,
    .label = wgpuStr("Default Shader"),
  };

  WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(m_wgpuDevice, &shaderDesc);

  // Configures z-buffer
  WGPUDepthStencilState depthStencilState {
    .nextInChain = nullptr,
    .format = WGPUTextureFormat_Depth24Plus,
    .depthWriteEnabled = WGPUOptionalBool_True,
    .depthCompare = WGPUCompareFunction_Less,
    .stencilFront {
      .compare = WGPUCompareFunction_Always,
      .failOp = WGPUStencilOperation_Keep,
      .depthFailOp = WGPUStencilOperation_Keep,
      .passOp = WGPUStencilOperation_Keep
    },
    .stencilBack {
      .compare = WGPUCompareFunction_Always,
      .failOp = WGPUStencilOperation_Keep,
      .depthFailOp = WGPUStencilOperation_Keep,
      .passOp = WGPUStencilOperation_Keep
    },
    .stencilReadMask = 0,
    .stencilWriteMask = 0,
    .depthBias = 0,
    .depthBiasSlopeScale = 0,
    .depthBiasClamp = 0,
  };

  WGPUBlendState blendState {
    .color {
      .operation = WGPUBlendOperation_Add,
      .srcFactor = WGPUBlendFactor_SrcAlpha,
      .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
    },
    .alpha {
      .operation = WGPUBlendOperation_Add,
      .srcFactor = WGPUBlendFactor_Zero,
      .dstFactor = WGPUBlendFactor_One,
    },
  };

  WGPUColorTargetState colorTarget {
    .nextInChain = nullptr,
    .format = m_wgpuTextureFormat,
    .blend = &blendState,
    .writeMask = WGPUColorWriteMask_All,
  };

  WGPUFragmentState fragState {
    .module = shaderModule,
    .entryPoint = wgpuStr("fsMain"),
    .constantCount = 0,
    .constants = nullptr,
    .targetCount = 1,
    .targets = &colorTarget,
  };

  std::vector<WGPUVertexAttribute> vertexAttributes;

  WGPUVertexAttribute posVertAttribute {
    .nextInChain = nullptr,
    .format = WGPUVertexFormat_Float32x3,
    .offset = 0,
    .shaderLocation = 0,
  };
  vertexAttributes.push_back(posVertAttribute);

  WGPUVertexAttribute uvXVertAttribute {
    .nextInChain = nullptr,
    .format = WGPUVertexFormat_Float32,
    .offset = sizeof(glm::vec3),
    .shaderLocation = 1,
  };
  vertexAttributes.push_back(uvXVertAttribute);

  WGPUVertexAttribute normVertAttribute {
    .nextInChain = nullptr,
    .format = WGPUVertexFormat_Float32x3,
    .offset =  sizeof(glm::vec3) + sizeof(float),
    .shaderLocation = 2,
  };
  vertexAttributes.push_back(normVertAttribute);

  WGPUVertexAttribute uvYVertAttribute {
    .nextInChain = nullptr,
    .format = WGPUVertexFormat_Float32,
    .offset =  sizeof(glm::vec3) * 2 + sizeof(float),
    .shaderLocation = 3,
  };
  vertexAttributes.push_back(uvYVertAttribute);


  WGPUVertexBufferLayout bufferLayout {
    .nextInChain = nullptr,
    .stepMode = WGPUVertexStepMode_Vertex,
    .arrayStride = sizeof(glm::vec3) * 2 + sizeof(float) * 2,
    .attributeCount = 4,
    .attributes = vertexAttributes.data(),
  };

  std::vector<WGPUBindGroupLayoutEntry> bindEntities;
  
  WGPUBindGroupLayoutEntry cameraBind = DefaultBindLayoutEntry();
  cameraBind.binding = 0;
  cameraBind.visibility = WGPUShaderStage_Vertex;
  cameraBind.buffer.type = WGPUBufferBindingType_Uniform;
  cameraBind.buffer.minBindingSize = sizeof(CameraData) + 4; // Adjusts for padding of vec3
  bindEntities.push_back( cameraBind );

  WGPUBindGroupLayoutEntry objDatBind = DefaultBindLayoutEntry();
  objDatBind.binding = 1;
  objDatBind.visibility = WGPUShaderStage_Vertex;
  objDatBind.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
  objDatBind.buffer.minBindingSize = sizeof(glm::mat4x4) + (sizeof(glm::vec4));
  bindEntities.push_back( objDatBind );

  WGPUBindGroupLayoutDescriptor bindLayoutDescriptor {
    .nextInChain = nullptr,
    .label = wgpuStr("Default Bind Layout"),
    .entryCount = bindEntities.size(), 
    .entries = bindEntities.data(),
  };

  WGPUBindGroupLayout bindLayout = wgpuDeviceCreateBindGroupLayout(m_wgpuDevice, &bindLayoutDescriptor);

  WGPUPipelineLayoutDescriptor pipelineLayoutConstructor {
    .nextInChain = nullptr,
    .label = wgpuStr("Base layout"),
    .bindGroupLayoutCount = 1,
    .bindGroupLayouts = &bindLayout,
  };

  WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(m_wgpuDevice, &pipelineLayoutConstructor);

  WGPURenderPipelineDescriptor pipelineDesc {
    .nextInChain = nullptr,
    .label = wgpuStr("Default Pipeline Layout"),
    .layout = pipelineLayout,
    .vertex {
      .module = shaderModule,
      .entryPoint = wgpuStr("vtxMain"),
      .constantCount = 0,
      .constants = nullptr,
      .bufferCount = 1,
      .buffers = &bufferLayout
    },
    .primitive {
      .topology = WGPUPrimitiveTopology_TriangleList,
      .stripIndexFormat = WGPUIndexFormat_Undefined,
      .frontFace = WGPUFrontFace_CCW,
      .cullMode = WGPUCullMode_None
    },
    .depthStencil = &depthStencilState,
    .multisample {
      .count = 1,
      .mask = ~0u,
      .alphaToCoverageEnabled = false,
    },
    .fragment = &fragState,
  };

  m_wgpuPipeline = wgpuDeviceCreateRenderPipeline(m_wgpuDevice, &pipelineDesc);

  WGPUBufferDescriptor cameraBufferDesc {
    .nextInChain = nullptr,
    .label = wgpuStr("Uniform Buffer Description"),
    .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
    .size = sizeof(CameraData) + 4, // Adjusts for padding
    .mappedAtCreation = false,
  };

  m_cameraBuffer = wgpuDeviceCreateBuffer(m_wgpuDevice, &cameraBufferDesc);

  WGPUBufferDescriptor storageBufferDesc {
    .nextInChain = nullptr,
    .label = wgpuStr("Storage Buffer Description"),
    .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage,
    .size = sizeof(ObjectData) * m_maxObjArraySize,
    .mappedAtCreation = false,
  };

  m_storageBuffer = wgpuDeviceCreateBuffer(m_wgpuDevice, &storageBufferDesc);

  std::vector<WGPUBindGroupEntry> bindGroupEntries;

  WGPUBindGroupEntry cameraBindEntry {
    .nextInChain = nullptr,
    .binding = 0,
    .buffer = m_cameraBuffer,
    .offset = 0,
    .size = sizeof(CameraData) + 4, // Adjusts for padding
  };
  bindGroupEntries.push_back(cameraBindEntry);


  WGPUBindGroupEntry objDataBindEntry {
    .nextInChain = nullptr,
    .binding = 1,
    .buffer = m_storageBuffer,
    .offset = 0,
    .size = sizeof(ObjectData) * m_maxObjArraySize
  };
  bindGroupEntries.push_back(objDataBindEntry);

  WGPUBindGroupDescriptor bindGroupDescriptor {
    .nextInChain = nullptr,
    .label = wgpuStr("Default Pipeline Bind Group"),
    .layout = bindLayout,
    .entryCount = bindGroupEntries.size(),
    .entries = bindGroupEntries.data(),
  };

  m_bindGroup = wgpuDeviceCreateBindGroup(m_wgpuDevice, &bindGroupDescriptor);

  SDL_free(loadedDat);
  wgpuPipelineLayoutRelease(pipelineLayout);
  wgpuShaderModuleRelease(shaderModule);
  wgpuBindGroupLayoutRelease(bindLayout);
}

MeshID WGPURenderBackend::UploadMesh(MeshAsset &asset) {
  return UploadMesh(asset.vertices.size(), asset.vertices.data(), asset.indices.size(), asset.indices.data());
}

MeshID WGPURenderBackend::UploadMesh(u32 vertCount, Vertex* vertices, u32 indexCount, u32* indices) {
  WGPUBufferDescriptor vertexBufferDesc {
    .nextInChain = nullptr,
    .label = wgpuStr("Mesh Vertex Buffer"),
    .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
    .size = sizeof(Vertex) * vertCount, // For now we only store vec3 positions
    .mappedAtCreation = false,
  };

  WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(m_wgpuDevice, &vertexBufferDesc);
  wgpuQueueWriteBuffer(m_wgpuQueue, vertexBuffer, 0, vertices, vertCount * sizeof(Vertex));

  WGPUBufferDescriptor indexBufferDesc {
    .nextInChain = nullptr,
    .label = wgpuStr("Mesh Vertex Buffer"),
    .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
    .size = sizeof(u32) * indexCount, // For now we only store vec3 positions
    .mappedAtCreation = false,
  };

  WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(m_wgpuDevice, &indexBufferDesc);
  wgpuQueueWriteBuffer(m_wgpuQueue, indexBuffer, 0, indices, indexCount * sizeof(u32));

  u32 retInt = m_nextMeshID;
  m_meshStore.emplace(std::pair<u32, Mesh>(retInt, Mesh(vertexBuffer,indexBuffer, indexCount, vertCount)));
  m_nextMeshID++;
  return retInt;
}

bool WGPURenderBackend::InitFrame() {
  #if SKL_ENABLED_EDITOR
  ImGui_ImplWGPU_NewFrame();
  #endif

  // Gets current color texture
  wgpuSurfaceGetCurrentTexture(m_wgpuSurface, &m_surfaceTexture);

  if (m_surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
      return false;
  }

  WGPUTextureViewDescriptor viewDescriptor {
    .nextInChain = nullptr,
    .label = wgpuStr("Surface texture view"),
    .format = wgpuTextureGetFormat(m_surfaceTexture.texture),
    .dimension = WGPUTextureViewDimension_2D,
    .baseMipLevel = 0,
    .mipLevelCount = 1,
    .baseArrayLayer = 0,
    .arrayLayerCount = 1,
    .aspect = WGPUTextureAspect_All,
    .usage = WGPUTextureUsage_RenderAttachment
  };

  m_surfaceTextureView = wgpuTextureCreateView(m_surfaceTexture.texture, &viewDescriptor);

  if(!m_surfaceTextureView)
  {
    return false;
  }

  // Creates a depth texture
  WGPUTextureDescriptor depthTextureDescriptor {
    .nextInChain = nullptr,
    .label = wgpuStr("Surface texture view"),
    .usage = WGPUTextureUsage_RenderAttachment,
    .dimension = WGPUTextureDimension_2D,
    .size = {m_screenWidth, m_screenHeight, 1},
    .format = m_wgpuDepthTextureFormat,
    .mipLevelCount = 1,
    .sampleCount = 1,
    .viewFormatCount = 1,
    .viewFormats = &m_wgpuDepthTextureFormat,
  };
  m_depthTexture = wgpuDeviceCreateTexture(m_wgpuDevice, &depthTextureDescriptor);

  WGPUTextureViewDescriptor depthViewDescriptor {
    .nextInChain = nullptr,
    .label = wgpuStr("Start depth view descriptor"),
    .format = m_wgpuDepthTextureFormat,
    .dimension = WGPUTextureViewDimension_2D,
    .baseMipLevel = 0,
    .mipLevelCount = 1,
    .baseArrayLayer = 0,
    .arrayLayerCount = 1,
    .aspect = WGPUTextureAspect_DepthOnly,
  };

  m_depthTextureView = wgpuTextureCreateView(m_depthTexture, &depthViewDescriptor);

  if(!m_depthTextureView) {
    wgpuTextureRelease(m_depthTexture);
    return false;
  }

  // Create a command encoder for the draw call
  WGPUCommandEncoderDescriptor encoderDesc = {
    .nextInChain = nullptr,
    .label = wgpuStr("Starting Encoder Descriptor")
  };
  WGPUCommandEncoder startCommandEncoder = wgpuDeviceCreateCommandEncoder(m_wgpuDevice, &encoderDesc);

  WGPURenderPassColorAttachment startPass {
    .view = m_surfaceTextureView,
    .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
    .resolveTarget = nullptr,
    .loadOp = WGPULoadOp_Clear,
    .storeOp = WGPUStoreOp_Store,
    .clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 }
  };

  WGPURenderPassDepthStencilAttachment depthStencilAttachment {
    .nextInChain = nullptr,
    .view = m_depthTextureView,
    .depthLoadOp = WGPULoadOp_Clear,
    .depthStoreOp = WGPUStoreOp_Store,
    .depthClearValue = 1.0f,
    .depthReadOnly = false,
    .stencilReadOnly = true,
  };

  WGPURenderPassDescriptor startPassDescriptor {
    .nextInChain = nullptr,
    .label = wgpuStr("Starting Render Pass Descriptor"),
    .colorAttachmentCount = 1,
    .colorAttachments = &startPass,
    .depthStencilAttachment = &depthStencilAttachment,
    .timestampWrites = nullptr,
  };

  WGPURenderPassEncoder startPassEncoder = wgpuCommandEncoderBeginRenderPass(startCommandEncoder, &startPassDescriptor);

  wgpuRenderPassEncoderEnd(startPassEncoder);
  wgpuRenderPassEncoderRelease(startPassEncoder);

  WGPUCommandBufferDescriptor cmdBufferDescriptor = {
    .nextInChain = nullptr,
    .label =  wgpuStr("Starting Command buffer"),
  };

  WGPUCommandBuffer startCommand = wgpuCommandEncoderFinish(startCommandEncoder, &cmdBufferDescriptor);
  wgpuCommandEncoderRelease(startCommandEncoder);

  wgpuQueueSubmit(m_wgpuQueue, 1, &startCommand);
  wgpuCommandBufferRelease(startCommand);

  return true;
}

void WGPURenderBackend::SetMesh(MeshID meshID) {
  if(m_doingColorPass) {
    EndMeshPass(); // Makes sure previous mesh pass ended before this one begins

    WGPUCommandEncoderDescriptor encoderDesc = {
      .nextInChain = nullptr,
      .label = wgpuStr("Mesh Encoder Descriptor")
    };
    m_meshCommandEncoder = wgpuDeviceCreateCommandEncoder(m_wgpuDevice, &encoderDesc);

    WGPURenderPassColorAttachment meshColorPass {
      .view = m_surfaceTextureView,
      .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
      .resolveTarget = nullptr,
      .loadOp = WGPULoadOp_Load,
      .storeOp = WGPUStoreOp_Store,
    };

    WGPURenderPassDepthStencilAttachment depthStencilAttachment {
      .nextInChain = nullptr,
      .view = m_depthTextureView,
      .depthLoadOp = WGPULoadOp_Load,
      .depthStoreOp = WGPUStoreOp_Store,
      .depthClearValue = 1.0f,
      .depthReadOnly = false,
      .stencilReadOnly = true,
    };

    WGPURenderPassDescriptor meshPassDesc {
      .nextInChain = nullptr,
      .label = wgpuStr("Basic mesh render pass"),
      .colorAttachmentCount = 1,
      .colorAttachments = &meshColorPass,
      .depthStencilAttachment = &depthStencilAttachment,
      .timestampWrites = nullptr,
    };

    m_meshPassEncoder = wgpuCommandEncoderBeginRenderPass(m_meshCommandEncoder, &meshPassDesc);
    wgpuRenderPassEncoderSetPipeline(m_meshPassEncoder, m_wgpuPipeline);
    wgpuRenderPassEncoderSetBindGroup(m_meshPassEncoder, 0, m_bindGroup, 0, nullptr);


    m_currentMeshID = meshID;
    wgpuRenderPassEncoderSetVertexBuffer(m_meshPassEncoder, 0, m_meshStore[meshID].m_vertexBuffer, 0, sizeof(Vertex) * m_meshStore[meshID].m_vertexCount);
    wgpuRenderPassEncoderSetIndexBuffer(m_meshPassEncoder,  m_meshStore[meshID].m_indexBuffer, WGPUIndexFormat_Uint32, 0, sizeof(u32) * m_meshStore[meshID].m_indexCount);

    m_meshBufferActive = true;
  }
}

void WGPURenderBackend::EndFrame() {
  EndMeshPass(); // Wraps up any loose mesh passes

  if (m_surfaceTextureView) {
    wgpuTextureViewRelease(m_surfaceTextureView);
  }

  if (m_depthTextureView) {
    wgpuTextureViewRelease(m_depthTextureView);
  }

  if (m_depthTexture) {
    wgpuTextureRelease(m_depthTexture);
  }

  #ifndef __EMSCRIPTEN__
  wgpuSurfacePresent(m_wgpuSurface);
  wgpuInstanceProcessEvents(m_wgpuInstance);  
  #else
    
  emscripten_sleep(10);
  #endif
}

void WGPURenderBackend::SendObjectData(std::vector<ObjectData>& objects) {
  wgpuQueueWriteBuffer(m_wgpuQueue, m_storageBuffer, 0, objects.data(), sizeof(ObjectData) * objects.size());
}

void WGPURenderBackend::DrawObjects(int count, int startIndex) {
  if(m_doingColorPass && m_meshBufferActive) {
    wgpuRenderPassEncoderDrawIndexed(m_meshPassEncoder, m_meshStore[m_currentMeshID].m_indexCount, count, 0, 0, startIndex);
  }
}

void WGPURenderBackend::BeginColorPass(CullMode cullMode) {
  m_doingColorPass = true;
  CameraData& gotCamera = m_cameraStore[m_currentCameraID];
  wgpuQueueWriteBuffer(m_wgpuQueue, m_cameraBuffer, 0, &gotCamera, sizeof(CameraData));

}

void WGPURenderBackend::EndPass() {
  m_doingColorPass = false;
}

CameraID WGPURenderBackend::AddCamera(u32 viewCount) {
  u32 retID = m_nextCameraID;
  m_cameraStore.emplace(std::pair<u32, CameraData>(retID, CameraData()));
  m_nextCameraID++;
  return retID;
}

void WGPURenderBackend::SetCamera(CameraID camera) {
  m_currentCameraID = camera;
  CameraData& gotCamera = m_cameraStore[m_currentCameraID];
  wgpuQueueWriteBuffer(m_wgpuQueue, m_cameraBuffer, 0, &gotCamera, sizeof(CameraData));
}

void WGPURenderBackend::UpdateCamera(u32 viewCount, CameraData* data) {
  m_cameraStore[m_currentCameraID] = *data;
  wgpuQueueWriteBuffer(m_wgpuQueue, m_cameraBuffer, 0, data, sizeof(CameraData));
}


void WGPURenderBackend::DrawImGui() {
  #if SKL_ENABLED_EDITOR
  EndMeshPass();

  WGPUCommandEncoderDescriptor encoderDesc = {
    .nextInChain = nullptr,
    .label = wgpuStr("Imgui Encoder Descriptor")
  };
  WGPUCommandEncoder imguiCommandEncoder = wgpuDeviceCreateCommandEncoder(m_wgpuDevice, &encoderDesc);

  WGPURenderPassColorAttachment meshColorPass {
    .view = m_surfaceTextureView,
    .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
    .resolveTarget = nullptr,
    .loadOp = WGPULoadOp_Load,
    .storeOp = WGPUStoreOp_Store,
  };

  WGPURenderPassDepthStencilAttachment depthStencilAttachment {
    .nextInChain = nullptr,
    .view = m_depthTextureView,
    .depthLoadOp = WGPULoadOp_Load,
    .depthStoreOp = WGPUStoreOp_Store,
    .depthClearValue = 1.0f,
    .depthReadOnly = false,
    .stencilReadOnly = true,
  };

  WGPURenderPassDescriptor meshPassDesc {
    .nextInChain = nullptr,
    .label = wgpuStr("Imgui render pass"),
    .colorAttachmentCount = 1,
    .colorAttachments = &meshColorPass,
    .depthStencilAttachment = &depthStencilAttachment,
    .timestampWrites = nullptr,
  };

  WGPURenderPassEncoder imguiPassEncoder = wgpuCommandEncoderBeginRenderPass(imguiCommandEncoder, &meshPassDesc);

  ImGui::Render();
  ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), imguiPassEncoder);

  wgpuRenderPassEncoderEnd(imguiPassEncoder);
  wgpuRenderPassEncoderRelease(imguiPassEncoder);

  WGPUCommandBufferDescriptor cmdBufferDescriptor = {
    .nextInChain = nullptr,
    .label =  wgpuStr("Imgui Command Buffer"),
  };

  WGPUCommandBuffer imguiCommand = wgpuCommandEncoderFinish(imguiCommandEncoder, &cmdBufferDescriptor);
  wgpuCommandEncoderRelease(imguiCommandEncoder);

  wgpuQueueSubmit(m_wgpuQueue, 1, &imguiCommand);
  wgpuCommandBufferRelease(imguiCommand);
  #endif
}
#pragma endregion
