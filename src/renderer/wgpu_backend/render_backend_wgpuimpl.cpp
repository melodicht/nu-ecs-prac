#include "renderer/render_backend.h"

#include "renderer/wgpu_backend/renderer_wgpu.h"

// This is done to force encapsulation of the wgpu renderer
static WGPURenderBackend wgpuRenderer;

// Uses composition to align more object oriented approach of web gpu backend to align with forward compilation 

SDL_WindowFlags GetRenderWindowFlags() {
    return wgpuRenderer.GetRenderWindowFlags();
}

void InitRenderer(SDL_Window *window, u32 startWidth, u32 startHeight) {
    wgpuRenderer.InitRenderer(window, startWidth, startHeight);
}

u32 UploadMesh(u32 vertCount, Vertex* vertices, u32 indexCount, u32* indices) {
    return wgpuRenderer.UploadMesh(vertCount, vertices, indexCount, indices);
}

u32 UploadMesh(MeshAsset &asset) {
    return wgpuRenderer.UploadMesh(asset);
}

CameraID AddCamera() { 
    return wgpuRenderer.AddCamera();
}

TextureID CreateDepthTexture(u32 width, u32 height) { 
    return wgpuRenderer.CreateDepthTexture(width, height); 
}

void DestroyTexture(TextureID textureID) { 
    wgpuRenderer.DestroyTexture(textureID);
}

void DestroyMesh(uint32_t meshID) {
    wgpuRenderer.DestroyMesh(meshID);
}

bool InitFrame() {
    return wgpuRenderer.InitFrame();
}

void SetCamera(CameraID camera) {
    wgpuRenderer.SetCamera(camera);
}

void SetDirLight(glm::mat4 lightSpace, glm::vec3 lightDir, TextureID texture) {
    wgpuRenderer.SetDirLight(lightSpace, lightDir, texture);
}

void UpdateCamera(glm::mat4 view, glm::mat4 proj, glm::vec3 pos) {
    wgpuRenderer.UpdateCamera(view, proj, pos);
}

void BeginDepthPass(CullMode cullMode) {
    wgpuRenderer.BeginDepthPass(cullMode);
}

void BeginDepthPass(TextureID target, CullMode cullMode) {
    wgpuRenderer.BeginDepthPass(target, cullMode);
}

void BeginColorPass(CullMode cullMode) {
    wgpuRenderer.BeginColorPass(cullMode);
}

void EndPass()
{
    wgpuRenderer.EndPass();
}

void DrawImGui()
{
    wgpuRenderer.DrawImGui();
}

void SetMesh(MeshID meshID) {
    wgpuRenderer.SetMesh(meshID);
}

void SendObjectData(std::vector<ObjectData>& objects) {
    wgpuRenderer.SendObjectData(objects);
}

void EndFrame() {
    wgpuRenderer.EndFrame();
}

void DrawObjects(int count, int startIndex) {
    wgpuRenderer.DrawObjects(count, startIndex);
}