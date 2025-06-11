#include "asset_types.h"

void GameInitialize(Scene &scene)
{
    bool slowStep = false;

    MeshAsset cuboidAsset = LoadMeshAsset("models/cube.glb");
    MeshAsset trapAsset = LoadMeshAsset("models/trap.glb");
    MeshAsset pyraAsset = LoadMeshAsset("models/pyra.glb");
    MeshAsset prismAsset = LoadMeshAsset("models/prism.glb");

    cuboidMesh = UploadMesh(cuboidAsset);
    trapMesh = UploadMesh(trapAsset);
    pyraMesh = UploadMesh(pyraAsset);
    prismMesh = UploadMesh(prismAsset);


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
    startCubeColor->r = RandInBetween(0.0f, 1.0f);
    startCubeColor->g = RandInBetween(0.0f, 1.0f);
    startCubeColor->b = RandInBetween(0.0f, 1.0f);

//    EntityID directionalLight = scene.NewEntity();
//    Transform3D* lightTransform = scene.Assign<Transform3D>(directionalLight);
//    lightTransform->rotation = {0, 30, 120};
//    DirLight* dirLight = scene.Assign<DirLight>(directionalLight);
//    dirLight->diffuse = {1, 1, 1};
//    dirLight->specular = {1, 1, 1};

//    EntityID pointLight = scene.NewEntity();
//    Transform3D* pointTransform = scene.Assign<Transform3D>(pointLight);
//    pointTransform->position.z = 640;
//    PointLight* pointLightComponent = scene.Assign<PointLight>(pointLight);
//    pointLightComponent->diffuse = {1, 1, 1};
//    pointLightComponent->specular = {1, 1, 1};
//    pointLightComponent->constant = 1;
//    pointLightComponent->linear = 0.0005;
//    pointLightComponent->quadratic = 0.00005;
//    pointLightComponent->maxRange = 1000;

    EntityID spotLight = scene.NewEntity();
    Transform3D* spotTransform = scene.Assign<Transform3D>(spotLight);
    spotTransform->position = {0, -256, 640};
    spotTransform->rotation = {0, 35, 50};
    SpotLight* spotLightComponent = scene.Assign<SpotLight>(spotLight);
    spotLightComponent->diffuse = {1, 1, 1};
    spotLightComponent->specular = {1, 1, 1};
    spotLightComponent->innerCone = 25;
    spotLightComponent->outerCone = 35;
    spotLightComponent->range = 1000;


    EntityID pointLightMarker = scene.NewEntity();
    MeshComponent* lightCube = scene.Assign<MeshComponent>(pointLightMarker);
    lightCube->mesh = cuboidMesh;
    ColorComponent* lightCubeColor = scene.Assign<ColorComponent>(pointLightMarker);
    lightCubeColor->r = RandInBetween(0.0f, 1.0f);
    lightCubeColor->g = RandInBetween(0.0f, 1.0f);
    lightCubeColor->b = RandInBetween(0.0f, 1.0f);
    Transform3D* pointCubeTransform = scene.Assign<Transform3D>(pointLightMarker);
    pointCubeTransform->position = spotTransform->position;
    pointCubeTransform->scale = {32, 32, 32};

    scene.InitSystems();
}

void GameUpdateAndRender(Scene &scene, SDL_Window *window, f32 deltaTime)
{
    scene.UpdateSystems(deltaTime);
}
