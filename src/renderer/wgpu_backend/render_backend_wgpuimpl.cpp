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

void InitPipelines(u32 numCascades) {
    wgpuRenderer.InitPipelines(numCascades);
}

u32 UploadMesh(u32 vertCount, Vertex* vertices, u32 indexCount, u32* indices) {
    return wgpuRenderer.UploadMesh(vertCount, vertices, indexCount, indices);
}

u32 UploadMesh(MeshAsset &asset) {
    return wgpuRenderer.UploadMesh(asset);
}

CameraID AddCamera(u32 viewCount) { 
    return wgpuRenderer.AddCamera(viewCount);
}

TextureID CreateDepthTexture(u32 width, u32 height) { 
    return wgpuRenderer.CreateDepthTexture(width, height); 
}

void DestroyTexture(TextureID textureID) { 
    wgpuRenderer.DestroyTexture(textureID);
}

void DestroyMesh(u32 meshID) {
    wgpuRenderer.DestroyMesh(meshID);
}

bool InitFrame() {
    return wgpuRenderer.InitFrame();
}

void SetCamera(CameraID camera) {
    wgpuRenderer.SetCamera(camera);
}

void SetDirLight(LightCascade* cascades, glm::vec3 lightDir, TextureID texture) {
    wgpuRenderer.SetDirLight(cascades, lightDir, texture);
}

void UpdateCamera(u32 viewCount, CameraData* views) {
    wgpuRenderer.UpdateCamera(viewCount, views);
}

void BeginDepthPass(CullMode cullMode) {
    wgpuRenderer.BeginDepthPass(cullMode);
}

void BeginShadowPass(TextureID target, CullMode cullMode) {
    wgpuRenderer.BeginShadowPass(target, cullMode);
}

void BeginCascadedPass(TextureID target, CullMode cullMode) {
    wgpuRenderer.BeginCascadedPass(target, cullMode);
}

void BeginColorPass(CullMode cullMode) {
    wgpuRenderer.BeginColorPass(cullMode);
}

void EndPass() {
    wgpuRenderer.EndPass();
}

void DrawImGui() {
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

TextureID CreateDepthArray(u32 width, u32 height, u32 layers) {
    return wgpuRenderer.CreateDepthArray(width, height, layers);
}