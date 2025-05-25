#include "renderer/wpu_backend/renderer_wpu.h"
#include "webgpu/sdl3webgpu-main/sdl3webgpu.h"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif

#include <cstdlib>
#include <cstring>
#include "skl_logger.h"
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
  WGPUSurfaceConfiguration config { 
    .nextInChain = nullptr,
    .device = m_wgpuDevice,
    .format = capabilities.formats[0],
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