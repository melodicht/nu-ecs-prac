#include "renderer/wpu_backend/renderer_wpu.h"
#include "webgpu/sdl3webgpu-main/sdl3webgpu.h"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif

#include <cstdlib>
#include <cstring>
#include "skl_logger.h"
#include <iostream>

WGPUStringView wgpuString(const char* string)
{
  WGPUStringView retString {
    .data = string,
    .length = std::strlen(string)
  };
  return retString;
}

void handle_request_adapter(WGPURequestAdapterStatus status,
  WGPUAdapter adapter, WGPUStringView message,
  void *userdata1, void *userdata2) {
  *(WGPUAdapter *)userdata1 = adapter;
}


std::pair<WGPUSurfaceTexture, WGPUTextureView> GetNextSurfaceViewData(WGPUSurface surface) {
  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);

  if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
      return { surfaceTexture, nullptr };
  }
  WGPUTextureViewDescriptor viewDescriptor;
  viewDescriptor.nextInChain = nullptr;
  viewDescriptor.label = wgpuString("Surface texture view");
  viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
  viewDescriptor.dimension = WGPUTextureViewDimension_2D;
  viewDescriptor.baseMipLevel = 0;
  viewDescriptor.mipLevelCount = 1;
  viewDescriptor.baseArrayLayer = 0;
  viewDescriptor.arrayLayerCount = 1;
  viewDescriptor.aspect = WGPUTextureAspect_All;
  WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

  return { surfaceTexture, targetView };
}

#pragma region Helper Functions
WGPUAdapter WPURenderBackend::GetAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options) {
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

WGPUDevice WPURenderBackend::GetDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
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

void WPURenderBackend::QueueFinishCallback(WGPUQueueWorkDoneStatus status, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
  LOG("Queued work finished with status: " << status);
}

void WPURenderBackend::LostDeviceCallback(WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
  LOG("Device lost: reason " << reason);
  if (message.data) LOG(" (" << message.data << ")");
}

void WPURenderBackend::ErrorCallback(WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
  LOG("Error happened: error " << type);
  if (message.data) LOG("(" << message.data << ")");
}
#pragma endregion
// We also add an inspect device function:
void inspectDevice(WGPUDevice device) {
  WGPUSupportedFeatures features;
  wgpuDeviceGetFeatures(device, &features);

  LOG("Device features:");
  for (int iter = 0; iter < features.featureCount ; iter++) {
      LOG(" - 0x" << features.features[iter]);
  }

  WGPULimits limits = {};
  limits.nextInChain = nullptr;

  WGPUStatus success = wgpuDeviceGetLimits(device, &limits);

  if (success == WGPUStatus_Success) {
      LOG("Device limits:");
      LOG(" - maxTextureDimension1D: " << limits.maxTextureDimension1D);
      LOG(" - maxTextureDimension2D: " << limits.maxTextureDimension2D);
      LOG(" - maxTextureDimension3D: " << limits.maxTextureDimension3D);
      LOG(" - maxTextureArrayLayers: " << limits.maxTextureArrayLayers);
      // [...] Extra device limits
  }
}

#pragma region Interface Impl
WPURenderBackend::~WPURenderBackend() {
  wgpuSurfaceUnconfigure(m_wgpuSurface);
  wgpuSurfaceRelease(m_wgpuSurface);
  wgpuQueueRelease(m_wgpuQueue);
  wgpuDeviceRelease(m_wgpuDevice);
  wgpuInstanceRelease(m_wgpuInstance);
}

// Much of this was taken from https://eliemichel.github.io/LearnWebGPU
void WPURenderBackend::InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight) {
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
    .label = wgpuString("My Device"),
    .requiredFeatureCount = 0,
    .requiredLimits = nullptr,
    .defaultQueue {
      .nextInChain = nullptr,
      .label = wgpuString("Default Queue")
    },
    .deviceLostCallbackInfo {
      .callback = LostDeviceCallback,
      .mode = WGPUCallbackMode_AllowSpontaneous
    },
    .uncapturedErrorCallbackInfo {
      .nextInChain = nullptr,
      .callback = ErrorCallback
    }
  };

  m_wgpuDevice = GetDevice(adapter, &deviceDesc);

  LOG("Got device: " << m_wgpuDevice);

  inspectDevice(m_wgpuDevice);

  m_wgpuQueue = wgpuDeviceGetQueue(m_wgpuDevice);

  WGPUQueueWorkDoneCallbackInfo queueDoneCallback =  WGPUQueueWorkDoneCallbackInfo {
    .callback = QueueFinishCallback,
    .mode = WGPUCallbackMode_AllowSpontaneous
  };

  wgpuQueueOnSubmittedWorkDone(m_wgpuQueue, queueDoneCallback);

  WGPUCommandEncoderDescriptor encoderDesc { 
    .nextInChain = nullptr,
    .label = wgpuString("My Encoder")
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
    .label = wgpuString("Command Buffer")
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
    .height = startHeight,
    .width = startWidth,
    .device = m_wgpuDevice,
    .usage = WGPUTextureUsage_RenderAttachment,
    .format = capabilities.formats[0],
    .presentMode = WGPUPresentMode_Fifo,
    .alphaMode = WGPUCompositeAlphaMode_Auto,
    .viewFormatCount = 0,
    .viewFormats = nullptr
  };
  wgpuSurfaceCapabilitiesFreeMembers( capabilities );

  wgpuSurfaceConfigure(m_wgpuSurface, &config);
  wgpuAdapterRelease(adapter);
}

bool WPURenderBackend::InitFrame() {
	// Create a command encoder for the draw call
	WGPUCommandEncoderDescriptor encoderDesc = {
    .nextInChain = nullptr,
    .label = wgpuString("My command encoder")
  };
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_wgpuDevice, &encoderDesc);

  auto viewData = GetNextSurfaceViewData(m_wgpuSurface);
  WGPUSurfaceTexture surfaceTexture = viewData.first;
  WGPUTextureView textureView = viewData.second;

  WGPURenderPassColorAttachment colorAttachment {
    .view = textureView,
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
	cmdBufferDescriptor.label = wgpuString("Command buffer");
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