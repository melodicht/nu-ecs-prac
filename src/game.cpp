#include "asset_types.h"

void GameInitialize(Scene &scene)
{
    bool slowStep = false;
    
    MeshAsset cuboidAsset = LoadMeshAsset("models/cube.glb");
    MeshAsset trapAsset = LoadMeshAsset("models/trap.glb");
    MeshAsset pyraAsset = LoadMeshAsset("models/pyra.glb");
    MeshAsset prismAsset = LoadMeshAsset("models/prism.glb");
    
    RenderUploadMeshInfo cuboidAssetDesc {
        .vertData = cuboidAsset.vertices.data(),
        .idxData = cuboidAsset.indices.data(),
        .vertSize = (u32)cuboidAsset.vertices.size(),
        .idxSize = (u32)cuboidAsset.indices.size()
    };

    RenderUploadMeshInfo trapAssetDesc {
        .vertData = trapAsset.vertices.data(),
        .idxData = trapAsset.indices.data(),
        .vertSize = (u32)trapAsset.vertices.size(),
        .idxSize = (u32)trapAsset.indices.size()
    };

    RenderUploadMeshInfo pyraAssetDesc {
        .vertData = pyraAsset.vertices.data(),
        .idxData = pyraAsset.indices.data(),
        .vertSize = (u32)pyraAsset.vertices.size(),
        .idxSize =(u32) pyraAsset.indices.size()
    };

    RenderUploadMeshInfo prismAssetDesc {
        .vertData = prismAsset.vertices.data(),
        .idxData = prismAsset.indices.data(),
        .vertSize = (u32)prismAsset.vertices.size(),
        .idxSize = (u32)prismAsset.indices.size()
    };

    cuboidMesh = UploadMesh(cuboidAssetDesc);
    trapMesh = UploadMesh(trapAssetDesc);
    pyraMesh = UploadMesh(pyraAssetDesc);
    prismMesh = UploadMesh(prismAssetDesc);


    RenderSystem *renderSys = new RenderSystem();
    MovementSystem *movementSys = new MovementSystem();
    BuilderSystem *builderSys = new BuilderSystem(slowStep);
    scene.AddSystem(renderSys);
    scene.AddSystem(movementSys);
    scene.AddSystem(builderSys);

    EntityID player = scene.NewEntity();
    Transform3D *playerTransform = scene.Assign<Transform3D>(player);
    CameraComponent *playerCamera = scene.Assign<CameraComponent>(player);
    FlyingMovement *playerMovement = scene.Assign<FlyingMovement>(player);
    playerCamera->fov = 90.0f;
    playerCamera->far = 5000.0f;
    playerCamera->near = 0.2f;
    playerMovement->moveSpeed = 200.0f;
    playerMovement->turnSpeed = 0.1f;
    playerTransform->position.x = -2560.0f;
    playerTransform->position.z = 1536.0f;
    playerTransform->rotation.y = 30.0f;

    EntityID startPlane = scene.NewEntity();
    scene.Assign<Transform3D>(startPlane);
    Plane *planeSize = scene.Assign<Plane>(startPlane);
    planeSize->width = 4096.0f;
    planeSize->length = 4096.0f;

    EntityID startCube = scene.NewEntity();
    Transform3D* cubeTransform = scene.Assign<Transform3D>(startCube);
    cubeTransform->scale.x = 4096.0f;
    cubeTransform->scale.y = 4096.0f;
    cubeTransform->scale.z = 16.0f;
    cubeTransform->position.z = -8.0f;
    MeshComponent* startCubeMesh = scene.Assign<MeshComponent>(startCube);
    startCubeMesh->mesh = cuboidMesh;
    ColorComponent* startCubeColor = scene.Assign<ColorComponent>(startCube);
    f32 shade = RandInBetween(0.25f, 0.75f);
    startCubeColor->r = shade;
    startCubeColor->g = shade;
    startCubeColor->b = shade;

    EntityID directionalLight = scene.NewEntity();
    Transform3D* lightTransform = scene.Assign<Transform3D>(directionalLight);
    lightTransform->rotation = {0, 30, 120};
    DirLight* dirLight = scene.Assign<DirLight>(directionalLight);
    dirLight->diffuse = {0.2, 0.2, 0.6};
    dirLight->specular = {0.4, 0.4, 0.5};


    scene.InitSystems();
}

void GameUpdateAndRender(Scene &scene, SDL_Window *window, f32 deltaTime)
{
    scene.UpdateSystems(deltaTime);
}
