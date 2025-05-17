#include "renderer_wpu.h"

#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include <cstdlib>
#include <iostream>

void WPURenderBackend::InitRenderer(SDL_Window *window) {
    wgpu::InstanceDescriptor instanceDescriptor{};
    instanceDescriptor.capabilities.timedWaitAnyEnable = true;
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    if (instance == nullptr) {
      std::cerr << "Instance creation failed!\n";
      return;
    }
    // Synchronously request the adapter.
    wgpu::RequestAdapterOptions options = {};
    wgpu::Adapter adapter;
  
    auto callback = [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, const char *message, void *userdata) {
      if (status != wgpu::RequestAdapterStatus::Success) {
        std::cerr << "Failed to get an adapter:" << message;
        return;
      }
      *static_cast<wgpu::Adapter *>(userdata) = adapter;
    };
  
  
    auto callbackMode = wgpu::CallbackMode::WaitAnyOnly;
    void *userdata = &adapter;
    instance.WaitAny(instance.RequestAdapter(&options, callbackMode, callback, userdata), UINT64_MAX);
    if (adapter == nullptr) {
      std::cerr << "RequestAdapter failed!\n";
      return;
    }
  
    wgpu::DawnAdapterPropertiesPowerPreference power_props{};
  
    wgpu::AdapterInfo info{};
    info.nextInChain = &power_props;
  
    adapter.GetInfo(&info);
    std::cout << "VendorID: " << std::hex << info.vendorID << std::dec << "\n";
    std::cout << "Vendor: " << info.vendor << "\n";
    std::cout << "Architecture: " << info.architecture << "\n";
    std::cout << "DeviceID: " << std::hex << info.deviceID << std::dec << "\n";
    std::cout << "Name: " << info.device << "\n";
    std::cout << "Driver description: " << info.description << "\n";
}