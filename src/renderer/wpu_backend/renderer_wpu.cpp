#include "renderer/wpu_backend/renderer_wpu.h"

#include <webgpu/webgpu.h>

#include <cstdlib>
#include <iostream>

void WPURenderBackend::InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight) {
  WGPUInstanceDescriptor instanceDescriptor{};

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
}