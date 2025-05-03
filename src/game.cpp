glm::vec4 cubeVerts[] =
{
    {-0.5f, -0.5f, -0.5f, 1.0f}, // 0: Back-Left-Bottom
    {0.5f, -0.5f, -0.5f, 1.0f},  // 1: Front-Left-Bottom
    {-0.5f, 0.5f, -0.5f, 1.0f},  // 2: Back-Right-Bottom
    {0.5f, 0.5f, -0.5f, 1.0f},   // 3: Front-Right-Bottom
    {-0.5f, -0.5f, 0.5f, 1.0f},  // 4: Back-Left-Top
    {0.5f, -0.5f, 0.5f, 1.0f},   // 5: Front-Left-Top
    {-0.5f, 0.5f, 0.5f, 1.0f},   // 6: Back-Right-Top
    {0.5f, 0.5f, 0.5f, 1.0f}     // 7: Front-Right-Top
};

glm::vec4 trapVerts[] =
{
    {-0.5f, -0.5f, -0.5f, 1.0f}, // 0: Back-Left-Bottom
    {0.5f, -0.5f, -0.5f, 1.0f},  // 1: Front-Left-Bottom
    {-0.5f, 0.5f, -0.5f, 1.0f},  // 2: Back-Right-Bottom
    {0.5f, 0.5f, -0.5f, 1.0f},   // 3: Front-Right-Bottom
    {-0.25f, -0.25f, 0.25f, 1.0f},  // 4: Back-Left-Top
    {0.25f, -0.25f, 0.25f, 1.0f},   // 5: Front-Left-Top
    {-0.25f, 0.25f, 0.25f, 1.0f},   // 6: Back-Right-Top
    {0.25f, 0.25f, 0.25f, 1.0f}     // 7: Front-Right-Top
};

u32 cubeTrapIndices[] =
{
    0, 1, 2,  // Bottom Face
    1, 2, 3,
    4, 5, 6,  // Top Face
    5, 6, 7,
    1, 3, 5,  // Front Face
    3, 5, 7,
    0, 2, 4,  // Back Face
    2, 4, 6,
    0, 1, 4,  // Left Face
    1, 4, 5,
    2, 3, 6,  // Right Face
    3, 6, 7
};

glm::vec4 pyraVerts[] =
{
    {-0.5f, -0.5f, -0.5f, 1.0f}, // 0: Back-Left-Bottom
    {0.5f, -0.5f, -0.5f, 1.0f},  // 1: Front-Left-Bottom
    {-0.5f, 0.5f, -0.5f, 1.0f},  // 2: Back-Right-Bottom
    {0.5f, 0.5f, -0.5f, 1.0f},   // 3: Front-Right-Bottom
    {0.0f, 0.0f, 0.5f, 1.0f}     // 4: Top
};

u32 pyraIndices[] =
{
    0, 1, 2,  // Bottom Face
    1, 2, 3,
    1, 3, 4,  // Front Face
    0, 2, 4,  // Back Face
    0, 1, 4,  // Left Face
    2, 3, 4   // Right Face
};

glm::vec4 prismVerts[] =
{
    {-0.5f, -0.5f, -0.5f, 1.0f}, // 0: Back-Left-Bottom
    {0.5f, -0.5f, -0.5f, 1.0f},  // 1: Front-Left-Bottom
    {-0.5f, 0.5f, -0.5f, 1.0f},  // 2: Back-Right-Bottom
    {0.5f, 0.5f, -0.5f, 1.0f},   // 3: Front-Right-Bottom
    {-0.5f, 0.0f, 0.5f, 1.0f},   // 4: Back-Top
    {0.5f, 0.0f, 0.5f, 1.0f}     // 5: Front-Top
};

u32 prismIndices[] =
{
    0, 1, 2,  // Bottom Face
    1, 2, 3,
    1, 3, 5,  // Front Face
    0, 2, 4,  // Back Face
    0, 1, 4,  // Left Face
    1, 4, 5,
    2, 3, 4,  // Right Face
    3, 4, 5   // Right Face
};


// Spawns a ball with the given radius in the given scene.
inline EntityID SpawnBall(Scene &scene, float radius, bool hasGravity, bool triangle)
{
    EntityID ball = scene.NewEntity();
    Transform3D *pBallTransform = scene.Assign<Transform3D>(ball);
    Rigidbody *pBallRb = scene.Assign<Rigidbody>(ball);
    CircleCollider *pBallCC = scene.Assign<CircleCollider>(ball);
    MeshComponent *pBallMesh = scene.Assign<MeshComponent>(ball);

    if (hasGravity)
    {
        GravityComponent *pGravityComponent = scene.Assign<GravityComponent>(ball);
        pGravityComponent->strength = 2.0f;
    }

    pBallTransform->position.y = RandInBetween((WINDOW_WIDTH / -2.0f) + radius, (WINDOW_WIDTH / 2.0f) - radius);
    pBallTransform->position.z = RandInBetween((WINDOW_HEIGHT / -2.0f) + radius, (WINDOW_HEIGHT / 2.0f) - radius);
    pBallTransform->scale = glm::vec3(10.0f);
    pBallRb->v_x = RandInBetween(0.1, 0.2);
    pBallRb->v_y = RandInBetween(0.1, 0.2);
    pBallCC->radius = radius;

    if (triangle)
    {
        pBallMesh->mesh = pyraMesh;
    }
    else
    {
        pBallMesh->mesh = prismMesh;
    }

    return ball;
}

void GameInitialize(Scene &scene)
{
    cuboidMesh = UploadMesh(8, &cubeVerts[0], 36, &cubeTrapIndices[0]);
    trapMesh = UploadMesh(8, &trapVerts[0], 36, &cubeTrapIndices[0]);
    pyraMesh = UploadMesh(5, &pyraVerts[0], 18, &pyraIndices[0]);
    prismMesh = UploadMesh(6, &prismVerts[0], 24, &prismIndices[0]);


    GravitySystem *gravitySys = new GravitySystem();
    CollisionSystem *collisionSys = new CollisionSystem();
    RenderSystem *renderSys = new RenderSystem();
    MovementSystem *movementSys = new MovementSystem();
    scene.AddSystem(gravitySys);
    scene.AddSystem(collisionSys);
    scene.AddSystem(renderSys);
    scene.AddSystem(movementSys);

    EntityID player = scene.NewEntity();
    Transform3D *playerTransform = scene.Assign<Transform3D>(player);
    CameraComponent *playerCamera = scene.Assign<CameraComponent>(player);
    FlyingMovement *playerMovement = scene.Assign<FlyingMovement>(player);
    playerCamera->fov = 90.0f;
    playerCamera->far = 5000.0f;
    playerCamera->near = 0.15f;
    playerMovement->moveSpeed = 100.0f;
    playerMovement->turnSpeed = 0.1f;
    playerTransform->position.x = WINDOW_HEIGHT / -2.0f;
    playerTransform->position.y = WINDOW_WIDTH / 2.0f;
    playerTransform->rotation.z = -45.0f;

    for (u32 i = 0; i < NUM_BALLS; i++)
    {
        SpawnBall(scene, BALL_RADIUS, i % 10 == 0, RandInBetween(0, 1) > 0.5);
    }

    scene.InitSystems();
}

void GameUpdateAndRender(Scene &scene, SDL_Window *window, f32 deltaTime)
{
    scene.UpdateSystems(deltaTime);
}
