#include "renderer/wpu_backend/renderer_wpu.h"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif

#include <cstdlib>
#include <iostream>

void handle_request_adapter(WGPURequestAdapterStatus status,
  WGPUAdapter adapter, WGPUStringView message,
  void *userdata1, void *userdata2) {
  *(WGPUAdapter *)userdata1 = adapter;
}

/**
 * Utility function to get a WebGPU adapter, so that
 *     WGPUAdapter adapter = requestAdapterSync(options);
 * is roughly equivalent to
 *     const adapter = await navigator.gpu.requestAdapter(options);
 */
WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options) {
  WGPUAdapter set = nullptr;
  bool requestEnded = false;

  WGPURequestAdapterCallbackInfo callbackInfo;
  callbackInfo.callback = [](WGPURequestAdapterStatus status, WGPUAdapterImpl* adapter, WGPUStringView message, void *userdata1, void *userdata2) 
  {
      if (status == WGPURequestAdapterStatus_Success) {
        *((WGPUAdapter *)userdata1) = adapter;
      } else {
          std::cout << "Could not get WebGPU adapter: " << (message.data) << std::endl;
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

WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
  WGPUDevice set = nullptr;
  bool requestEnded = false;

  WGPURequestDeviceCallbackInfo callbackInfo;
  callbackInfo.callback = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void * userdata1, void * userdata2) {
      if (status == WGPURequestDeviceStatus_Success) {
        *((WGPUDevice *)userdata1) = device;
      } else {
          std::cout << "Could not get WebGPU device: " << message.data << std::endl;
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

// We also add an inspect device function:
void inspectDevice(WGPUDevice device) {
  WGPUSupportedFeatures features;
  wgpuDeviceGetFeatures(device, &features);

  std::cout << "Device features:" << std::endl;
  std::cout << std::hex;
  for (int iter = 0; iter < features.featureCount ; iter++) {
      std::cout << " - 0x" << features.features[iter] << std::endl;
  }
  std::cout << std::dec;

  WGPULimits limits = {};
  limits.nextInChain = nullptr;

  WGPUStatus success = wgpuDeviceGetLimits(device, &limits);

  if (success == WGPUStatus_Success) {
      std::cout << "Device limits:" << std::endl;
      std::cout << " - maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
      std::cout << " - maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
      std::cout << " - maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
      std::cout << " - maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
      // [...] Extra device limits
  }
}
// Much of this was taken from https://eliemichel.github.io/LearnWebGPU
void WPURenderBackend::InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight) {
  // Creates instance
  WGPUInstanceDescriptor instanceDescriptor{};
  instanceDescriptor.nextInChain = nullptr;

  #if EMSCRIPTEN
  WGPUInstance instance = wgpuCreateInstance(nullptr);
  #else
  WGPUInstance instance = wgpuCreateInstance(&instanceDescriptor);
  #endif
  if (instance == nullptr) {
    std::cerr << "Instance creation failed!\n";
    return;
  }
  std::cout << "Instance Created" << instance << std::endl;

  std::cout << "Requesting adapter..." << std::endl;

  WGPURequestAdapterOptions adapterOpts = {};
  adapterOpts.nextInChain = nullptr;
  WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);

  std::cout << "Got adapter: " << adapter << std::endl;

  std::cout << "Requesting device..." << std::endl;

  WGPUDeviceDescriptor deviceDesc = {};

  // General device description
  deviceDesc.nextInChain = nullptr;
  deviceDesc.label = WGPUStringView{
    .data = "My Device",
    .length = 9}; // anything works here, that's your call
  deviceDesc.requiredFeatureCount = 0; // we do not require any specific feature
  deviceDesc.requiredLimits = nullptr; // we do not require any specific limit
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.defaultQueue.label = WGPUStringView{
    .data = "Default Queue",
    .length = 13};

  // Lost device callback
  WGPUDeviceLostCallbackInfo lostCallbackInfo;
  lostCallbackInfo.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
    std::cout << "Device lost: reason " << reason;
    if (message.data) std::cout << " (" << message.data << ")";
    std::cout << std::endl;
  };
  lostCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  deviceDesc.deviceLostCallbackInfo = lostCallbackInfo;

  // Error callback
  WGPUUncapturedErrorCallbackInfo lostErrorInfo;
  lostErrorInfo.callback = [](WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
    std::cout << "Error happened: error " << type;
    if (message.data) std::cout << " (" << message.data << ")";
    std::cout << std::endl;
  };
  deviceDesc.uncapturedErrorCallbackInfo = lostErrorInfo;

  WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);

  std::cout << "Got device: " << device << std::endl;

  wgpuAdapterRelease(adapter);

  inspectDevice(device);

  wgpuDeviceRelease(device);
}