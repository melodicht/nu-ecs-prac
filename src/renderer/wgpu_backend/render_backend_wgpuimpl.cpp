#include "renderer/render_backend.h"

#include <map>
#include <random>

#include "renderer/wgpu_backend/renderer_wgpu.h"
#include "math/math_utils.cpp"


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
    return wgpuRenderer.UploadMesh(desc.meshAsset);
}

void DestroyMesh(RenderDestroyMeshDescriptor& desc) {
    wgpuRenderer.DestroyMesh(desc.meshID);
}

// This compiles information from scene to be plugged into renderer
void RenderUpdate(RenderUpdateDescriptor& desc) {
    WGPURenderState sendState;
    Scene* scene = desc.scene;

    // 1. Gather counts of each unique mesh pointer.
    sendState.m_meshCounts = std::map<MeshID, u32>();
    for (EntityID ent: SceneView<MeshComponent, ColorComponent, Transform3D>(*scene))
    {
        MeshComponent *m = scene->Get<MeshComponent>(ent);
        ++sendState.m_meshCounts[m->mesh];
    }

    // 2. Create, with fixed size, the list of Mat4s, by adding up all of the counts.
    // 3. Get pointers to the start of each segment of unique mesh pointer.
    u32 totalCount = 0;
    std::unordered_map<MeshID, u32> offsets;
    for (std::pair<MeshID, u32> pair: sendState.m_meshCounts)
    {
        offsets[pair.first] = totalCount;
        totalCount += pair.second;
    }

    sendState.m_objData = std::vector<WGPUObjectData>(totalCount);

    // 4. Iterate through scene view once more and fill in the fixed size array.
    for (EntityID ent: SceneView<MeshComponent, ColorComponent, Transform3D>(*scene))
    {
        Transform3D *t = scene->Get<Transform3D>(ent);
        glm::mat4 model = GetTransformMatrix(t);
        MeshComponent *m = scene->Get<MeshComponent>(ent);
        MeshID mesh = m->mesh;
        ColorComponent *c = scene->Get<ColorComponent>(ent);

        sendState.m_objData[offsets[mesh]++] = {model, glm::vec4(c->r, c->g, c->b, 1.0f)};
    }

    // Finds main camera view
    SceneView<CameraComponent, Transform3D> cameraView = SceneView<CameraComponent, Transform3D>(*scene);
    if (cameraView.begin() == cameraView.end())
    {
        return;
    }

    EntityID cameraEnt = *cameraView.begin();
    CameraComponent *camera = scene->Get<CameraComponent>(cameraEnt);
    Transform3D *cameraTransform = scene->Get<Transform3D>(cameraEnt);
    glm::mat4 view = GetViewMatrix(cameraTransform);
    f32 aspect = (f32)desc.screenWidth / (f32)desc.screenHeight;

    glm::mat4 proj = glm::perspective(glm::radians(camera->fov), aspect, camera->near, camera->far);

    sendState.m_mainCam = {view, proj, cameraTransform->position};

    // Finally sends compiled scene information to be processed by renderer
    wgpuRenderer.RenderUpdate(sendState);
}