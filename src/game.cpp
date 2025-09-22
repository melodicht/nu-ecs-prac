#include "game.h"

#include <map>
#include <random>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "asset_types.h"
#include "asset_utils.cpp"
#include "renderer/render_backend.h"

#include "ecs.cpp"

#include "math/skl_math_utils.h"

#include "physics.cpp"
#include "systems.cpp"

local void LogDebugRecords();

extern "C"
#if defined(_WIN32) || defined(_WIN64)
__declspec(dllexport)
#endif
GAME_INITIALIZE(GameInitialize)
{
    scene.getComponentStringId = memory.getStringId;
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

    // NOTE(marvin): Initialising the physics system.
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    // NOTE(marvin): This is not our ECS system! Jolt happened to name it System as well. 
    JPH::PhysicsSystem *physicsSystem = new JPH::PhysicsSystem();

    // NOTE(marvin): Pulled these numbers out of my ass.
    u32 maxBodies = 1024;
    u32 numBodyMutexes = 0;  // 0 means auto-detect.
    u32 maxBodyPairs = 1024;
    u32 maxContactConstraints = 1024;
    JPH::BroadPhaseLayerInterface *sklBroadPhaseLayer = new SklBroadPhaseLayer();
    JPH::ObjectVsBroadPhaseLayerFilter *sklObjectVsBroadPhaseLayerFilter = new SklObjectVsBroadPhaseLayerFilter();
    JPH::ObjectLayerPairFilter *sklObjectLayerPairFilter = new SklObjectLayerPairFilter();
    
    physicsSystem->Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints,
                        *sklBroadPhaseLayer, *sklObjectVsBroadPhaseLayerFilter,
                        *sklObjectLayerPairFilter);

    CharacterControllerSystem *characterControllerSys = new CharacterControllerSystem(physicsSystem);
    scene.AddSystem(characterControllerSys);

    RenderSystem *renderSys = new RenderSystem();
    MovementSystem *movementSys = new MovementSystem();
    BuilderSystem *builderSys = new BuilderSystem(slowStep);
    scene.AddSystem(renderSys);
    scene.AddSystem(movementSys);
    scene.AddSystem(builderSys);

    EntityID playerCharacterEnt = scene.NewEntity();
    Transform3D *pcTransform = scene.Assign<Transform3D>(playerCharacterEnt);
    PlayerCharacter *playerCharacter = scene.Assign<PlayerCharacter>(playerCharacterEnt);

    JPH::CharacterVirtualSettings characterVirtualSettings;
    f32 halfHeightOfCylinder = 1.0f;
    f32 cylinderRadius = 0.3f;
    characterVirtualSettings.mShape = new JPH::CapsuleShape(halfHeightOfCylinder, cylinderRadius);
    characterVirtualSettings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -cylinderRadius);
    
    JPH::Vec3 characterPosition = JPH::Vec3(0, 10, 0);  // Just so they are not stuck in the ground.
    JPH::Quat characterRotation = JPH::Quat(0, 0, 0, 0);
    JPH::CharacterVirtual *characterVirtual = new JPH::CharacterVirtual(&characterVirtualSettings, characterPosition, characterRotation, physicsSystem);
    playerCharacter->characterVirtual = characterVirtual;

    // NOTE(marvin): This is the freecam player, not to be confused with the player character.
    // Probably should give it a different name but I am lazy.
    EntityID player = scene.NewEntity();
    Transform3D *playerTransform = scene.Assign<Transform3D>(player);
    CameraComponent *playerCamera = scene.Assign<CameraComponent>(player);
    FlyingMovement *playerMovement = scene.Assign<FlyingMovement>(player);
    playerCamera->fov = 90.0f;
    playerCamera->farxx = 5000.0f;
    playerCamera->nearxx = 0.2f;
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

// NOTE(marvin): Our logger doesn't have string format...
// Using c std lib's one for now.
#if SKL_INTERNAL
#include <cstdio>
#endif


extern "C"
#if defined(_WIN32) || defined(_WIN64)
__declspec(dllexport)
#endif
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    scene.UpdateSystems(&input, deltaTime);
    LogDebugRecords();
}

// NOTE(marvin): This has to go after ALL the timed blocks in order of
// what the preprocesser sees, so that the counter here will be the
// number of all the timed blocks that it has seen.
DebugRecord debugRecordArray[__COUNTER__];

local void LogDebugRecords()
{
#if SKL_INTERNAL
    for (u32 i = 0;
         i < ArrayCount(debugRecordArray);
         ++i)
    {
        DebugRecord *debugRecord = debugRecordArray + i;

        u64 hitCount_cycleCount = AtomicExchangeU64(&debugRecord->hitCount_cycleCount, 0);
        u32 hitCount = (u32)(hitCount_cycleCount >> 32);
        u32 cycleCount = (u32)(hitCount_cycleCount & 0xFFFFFFFF);

        printf("%s:%s:%u %ucy (%uh) %ucy/h\n",
               debugRecord->blockName,
               debugRecord->fileName,
               debugRecord->lineNumber,
               cycleCount,
               hitCount,
               cycleCount / hitCount);
    }
    puts("\n");
#endif
}
