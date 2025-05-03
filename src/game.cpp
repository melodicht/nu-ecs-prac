glm::vec4 squareVertices[] =
{
    {0.0f, -5.0f, -5.0f, 1.0f},
    {0.0f, 5.0f, -5.0f, 1.0f},
    {0.0f, -5.0f, 5.0f, 1.0f},
    {0.0f, 5.0f, 5.0f, 1.0f}
};

u32 squareIndices[] =
{
    0, 1, 2,
    1, 2, 3,
};

glm::vec4 triangleVertices[] =
{
    {0.0f, -5.0f, -5.0f, 1.0f},
    {0.0f, 5.0f, -5.0f, 1.0f},
    {0.0f, 0.0f, 5.0f, 1.0f}
};

u32 triangleIndices[] =
{
    0, 1, 2,
};

Mesh* squareMesh = nullptr;
Mesh* triangleMesh = nullptr;

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
    pBallRb->v_x = RandInBetween(0.1, 0.2);
    pBallRb->v_y = RandInBetween(0.1, 0.2);
    pBallCC->radius = radius;

    if (triangle)
    {
        pBallMesh->mesh = triangleMesh;
    }
    else
    {
        pBallMesh->mesh = squareMesh;
    }

    return ball;
}

void GameInitialize(Scene &scene)
{
    squareMesh = UploadMesh(4, &squareVertices[0], 6, &squareIndices[0]);
    triangleMesh = UploadMesh(3, &triangleVertices[0], 3, &triangleIndices[0]);

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
