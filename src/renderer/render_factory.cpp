#define RENDER_BACKEND 1

#if RENDER_BACKEND == 0
    #include "renderer_vk.h"
#elif RENDER_BACKEND == 1
    #include "renderer_wpu.h"
#endif

std::unique_ptr<IRenderBackend> BuildRenderer() {
#if RENDER_BACKEND == 0
    return std::make_unique<VKRenderBackend>();
#elif RENDER_BACKEND == 1
    return std::make_unique<WPURenderBackend>();
#endif
}