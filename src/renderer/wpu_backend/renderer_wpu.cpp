#include "renderer/wpu_backend/renderer_wpu.h"
#include "webgpu/sdl3webgpu-main/sdl3webgpu.h"

#include "skl_logger.h"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>

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
    .binding = 0,
    .nextInChain = nullptr,
    .visibility = WGPUShaderStage_None,
    .buffer {
      .nextInChain = nullptr,
      .type = WGPUBufferBindingType_Undefined,
      .minBindingSize = 0,
      .hasDynamicOffset = false,
    },
    .sampler {
      .nextInChain = nullptr,
      .type = WGPUSamplerBindingType_BindingNotUsed,
    },
    .storageTexture {
      .nextInChain = nullptr,
      .access = WGPUStorageTextureAccess_BindingNotUsed,
      .format = WGPUTextureFormat_Undefined,
      .viewDimension = WGPUTextureViewDimension_Undefined,
    },
    .texture {
      .nextInChain = nullptr,
      .multisampled = false,
      .sampleType = WGPUTextureSampleType_BindingNotUsed,
      .viewDimension =  WGPUTextureViewDimension_Undefined,
    },
  };
}

WGPURenderPipeline WGPURenderBackend::CreateDefaultPipeline() {
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
    .depthCompare = WGPUCompareFunction_Less,
    .depthWriteEnabled = WGPUOptionalBool_True,
    .format = WGPUTextureFormat_Depth24Plus,
    .stencilReadMask = 0,
    .stencilWriteMask = 0,
    .depthBias = 0,
    .depthBiasSlopeScale = 0,
    .depthBiasClamp = 0,
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
  };

  WGPUBlendState blendState {
    .alpha {
      .srcFactor = WGPUBlendFactor_Zero,
      .dstFactor = WGPUBlendFactor_One,
      .operation = WGPUBlendOperation_Add,
    },
    .color {
      .srcFactor = WGPUBlendFactor_SrcAlpha,
      .operation = WGPUBlendOperation_Add,
      .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
    }
  };

  WGPUColorTargetState colorTarget {
    .blend = &blendState,
    .format = m_wgpuTextureFormat,
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

  WGPUVertexAttribute vertexAttribute {
    .format = WGPUVertexFormat_Float32x3,
    .shaderLocation = 0,
    .offset = 0,
  };

  WGPUVertexBufferLayout bufferLayout {
    .attributeCount = 1,
    .attributes = &vertexAttribute,
    .arrayStride = sizeof(glm::vec3),
    .stepMode = WGPUVertexStepMode_Vertex,
  };

  std::vector<WGPUBindGroupLayoutEntry> bindEntities;
  
  
  WGPUBindGroupLayoutEntry cameraBind = DefaultBindLayoutEntry();
  cameraBind.binding = 0;
  cameraBind.visibility = WGPUShaderStage_Vertex;
  cameraBind.buffer.type = WGPUBufferBindingType_Uniform;
  cameraBind.buffer.minBindingSize = sizeof(glm::mat4x4) * 2;
  bindEntities.push_back( cameraBind );

  WGPUBindGroupLayoutEntry objDatBind = DefaultBindLayoutEntry();
  objDatBind.binding = 1;
  objDatBind.visibility = WGPUShaderStage_Vertex;
  objDatBind.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
  objDatBind.buffer.minBindingSize = sizeof(glm::mat4x4) + (sizeof(glm::vec3) + 4); // Makes sure to adjust for padding of vec3
  bindEntities.push_back( objDatBind );

  WGPUBindGroupLayoutEntry primaryIndexBind = DefaultBindLayoutEntry();
  primaryIndexBind.binding = 2;
  primaryIndexBind.visibility = WGPUShaderStage_Vertex;
  primaryIndexBind.buffer.type = WGPUBufferBindingType_Uniform;
  primaryIndexBind.buffer.minBindingSize = sizeof(u32);
  bindEntities.push_back( primaryIndexBind );

  WGPUBindGroupLayoutEntry instanceIndexBind = DefaultBindLayoutEntry();
  instanceIndexBind.binding = 3;
  instanceIndexBind.visibility = WGPUShaderStage_Vertex;
  instanceIndexBind.buffer.type = WGPUBufferBindingType_Uniform;
  instanceIndexBind.buffer.minBindingSize = sizeof(u32);
  bindEntities.push_back( instanceIndexBind );

  WGPUBindGroupLayoutDescriptor bindLayoutDescriptor {
    .entryCount = bindEntities.size(), 
    .entries = bindEntities.data(),
    .label = wgpuStr("Default Bind Layout"),
    .nextInChain = nullptr,
  };

  WGPUBindGroupLayout bindLayout = wgpuDeviceCreateBindGroupLayout(m_wgpuDevice, &bindLayoutDescriptor);

  WGPUPipelineLayoutDescriptor pipelineLayoutConstructor {
    .bindGroupLayoutCount = 1,
    .bindGroupLayouts = &bindLayout,
    .nextInChain = nullptr,
    .label = wgpuStr("Base layout")
  };

  WGPURenderPipelineDescriptor pipelineDesc {
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
    .multisample {
      .count = 1,
      .mask = ~0u,
      .alphaToCoverageEnabled = false,
    },
    .fragment = &fragState,
    .depthStencil = &depthStencilState,
    .layout = wgpuDeviceCreatePipelineLayout(m_wgpuDevice, &pipelineLayoutConstructor),
  };

  return wgpuDeviceCreateRenderPipeline(m_wgpuDevice, &pipelineDesc);
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

void WGPURenderBackend::QueueFinishCallback(WGPUQueueWorkDoneStatus status, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
  LOG("Queued work finished with status: " << status);
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
    .mode = WGPUCallbackMode_AllowSpontaneous,
    .callback = QueueFinishCallback
  };

  wgpuQueueOnSubmittedWorkDone(m_wgpuQueue, queueDoneCallback);

  WGPUCommandEncoderDescriptor encoderDesc { 
    .nextInChain = nullptr,
    .label = wgpuStr("My Encoder")
  };

  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_wgpuDevice, &encoderDesc);

  wgpuCommandEncoderInsertDebugMarker(
    encoder,
    WGPUStringView{
      .data = "Do One Thing",
      .length = 12
    }
  );
  wgpuCommandEncoderInsertDebugMarker(
    encoder, 
    WGPUStringView{
      .data = "Do Another Thing",
      .length = 16
    }
  );

  WGPUCommandBufferDescriptor cmdBufferDescriptor { 
    .nextInChain = nullptr,
    .label = wgpuStr("Command Buffer")
  };

  WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
  wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

  // Finally submit the command queue
  LOG("Submitting command...");

  wgpuQueueSubmit(m_wgpuQueue, 1, &command);
  wgpuCommandBufferRelease(command);
  LOG("Command submitted.");

  for (int i = 0 ; i < 5 ; ++i) {
    LOG("Tick/Poll device...");
    #if EMSCRIPTEN
    emscripten_sleep(100);
    #else
    wgpuInstanceProcessEvents(m_wgpuInstance);  
    #endif
  }
  
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

  m_wgpuPipeline = CreateDefaultPipeline();
}

bool WGPURenderBackend::InitFrame() {
  WGPUSurfaceTexture surfaceTexture;
  WGPUTextureView textureView = nullptr;
  wgpuSurfaceGetCurrentTexture(m_wgpuSurface, &surfaceTexture);

  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
      return false;
  }

  WGPUTextureViewDescriptor viewDescriptor {
    .nextInChain = nullptr,
    .label = wgpuStr("Surface texture view"),
    .format = wgpuTextureGetFormat(surfaceTexture.texture),
    .dimension = WGPUTextureViewDimension_2D,
    .baseMipLevel = 0,
    .mipLevelCount = 1,
    .baseArrayLayer = 0,
    .arrayLayerCount = 1,
    .aspect = WGPUTextureAspect_All,
    .usage = WGPUTextureUsage_RenderAttachment
  };

  textureView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

  if(!textureView)
  {
    return false;
  }

	// Create a command encoder for the draw call
	WGPUCommandEncoderDescriptor encoderDesc = {
    .nextInChain = nullptr,
    .label = wgpuStr("My command encoder")
  };
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_wgpuDevice, &encoderDesc);

  WGPURenderPassColorAttachment colorAttachment {
    .view = textureView,
    .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
    .resolveTarget = nullptr,
    .loadOp = WGPULoadOp_Clear,
    .storeOp = WGPUStoreOp_Store,
    .clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 }
  };

  WGPURenderPassDescriptor renderPassDescriptor {
    .nextInChain = nullptr,
    .colorAttachmentCount = 1,
    .colorAttachments = &colorAttachment,
    .depthStencilAttachment = nullptr,
    .timestampWrites = nullptr
  };

  WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescriptor);
  wgpuRenderPassEncoderEnd(renderPass);
	wgpuRenderPassEncoderRelease(renderPass);

  WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.nextInChain = nullptr;
	cmdBufferDescriptor.label = wgpuStr("Command buffer");
	WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
	wgpuCommandEncoderRelease(encoder);

  wgpuQueueSubmit(m_wgpuQueue, 1, &command);
	wgpuCommandBufferRelease(command);

  if(textureView) {
    wgpuTextureViewRelease(textureView);
  }

#ifndef __EMSCRIPTEN__
  wgpuSurfacePresent(m_wgpuSurface);
  wgpuInstanceProcessEvents(m_wgpuInstance);  
#else
  
  emscripten_sleep(100);
#endif
  return true;
}
#pragma endregion