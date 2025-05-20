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

  // Callback called by wgpuInstanceRequestAdapter when the request returns
  // This is a C++ lambda function, but could be any function defined in the
  // global scope. It must be non-capturing (the brackets [] are empty) so
  // that it behaves like a regular C function pointer, which is what
  // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
  // is to convey what we want to capture through the pUserData pointer,
  // provided as the last argument of wgpuInstanceRequestAdapter and received
  // by the callback as its last argument.
  WGPURequestAdapterCallbackInfo callbackInfo = WGPURequestAdapterCallbackInfo();
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
  while (!userData.requestEnded) {
      emscripten_sleep(100);
  }
  #endif

  assert(requestEnded);

  return set;
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
}