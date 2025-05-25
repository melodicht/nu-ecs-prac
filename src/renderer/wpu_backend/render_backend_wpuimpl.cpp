#include "renderer/render_backend.h"

#include "renderer/wpu_backend/renderer_wpu.h"

static WGPURenderBackend wpu_renderer;

// Uses composition to align more object oriented approach of web gpu backend to align with forward compilation 

SDL_WindowFlags GetRenderWindowFlags() {
    return wpu_renderer.GetRenderWindowFlags();
}

void InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight) {
    wpu_renderer.InitRenderer(window, startWidth, startHeight);
}

uint32_t UploadMesh(uint32_t vertCount, Vertex* vertices, uint32_t indexCount, uint32_t* indices) {
    return wpu_renderer.UploadMesh(vertCount, vertices, indexCount, indices);
}

uint32_t UploadMesh(MeshAsset &asset) {
    return wpu_renderer.UploadMesh(asset);
}

void DestroyMesh(uint32_t meshID) {
    wpu_renderer.DestroyMesh(meshID);
}

bool InitFrame() {
    return wpu_renderer.InitFrame();
}

void SetCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos) {
    wpu_renderer.SetCamera(view, proj, pos);
}

void SetMesh(uint32_t meshID) {
    wpu_renderer.SetMesh(meshID);
}

void SendObjectData(std::vector<ObjectData>& objects) {
    wpu_renderer.SendObjectData(objects);
}

void EndFrame() {
    wpu_renderer.EndFrame();
}

void DrawObjects(int count, int startIndex) {
    wpu_renderer.DrawObjects(count, startIndex);
}