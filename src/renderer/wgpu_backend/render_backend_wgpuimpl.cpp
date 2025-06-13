#include "renderer/render_backend.h"

#include <map>
#include <random>

#include "renderer/wgpu_backend/renderer_wgpu.h"


// This is done to force encapsulation of the wgpu renderer and renderer types
static WGPURenderBackend wgpuRenderer;

SDL_WindowFlags GetRenderWindowFlags() {
    return 0;
}

void InitRenderer(RenderInitDescriptor& desc) {
    wgpuRenderer.InitRenderer(desc.window, desc.startWidth, desc.startHeight);
}

void InitPipelines(RenderPipelineInitDescriptor& desc) {
    wgpuRenderer.InitPipelines();
}

MeshID UploadMesh(RenderUploadMeshDescriptor& desc) {
    return wgpuRenderer.UploadMesh(desc.vertSize, desc.vertData, desc.idxSize, desc.idxData);
}

void DestroyMesh(RenderDestroyMeshDescriptor& desc) {
    wgpuRenderer.DestroyMesh(desc.meshID);
}

// This compiles information from scene to be plugged into renderer
void RenderUpdate(RenderFrameState& state) {
    wgpuRenderer.RenderUpdate(state);
}
