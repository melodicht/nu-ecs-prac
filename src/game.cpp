// Spawns a ball with the given radius in the given scene.
inline EntityID
SpawnBall(Scene &scene, float radius, bool hasGravity)
{
    EntityID ball = scene.NewEntity();
    Transform3D *pBallTransform = scene.Assign<Transform3D>(ball);
    Rigidbody *pBallRb = scene.Assign<Rigidbody>(ball);
    CircleCollider *pBallCC = scene.Assign<CircleCollider>(ball);
    ColorComponent *pColorComponent = scene.Assign<ColorComponent>(ball);

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
    pColorComponent->r = (u32) RandInBetween(50, 256);
    pColorComponent->g = (u32) RandInBetween(50, 256);
    pColorComponent->b = (u32) RandInBetween(50, 256);
    return ball;
}

void GameInitialize(Scene &scene)
{
    GravitySystem *gravitySys = new GravitySystem();
    CollisionSystem *collisionSys = new CollisionSystem();
    RenderSystem *renderSys = new RenderSystem();
    scene.AddSystem(gravitySys);
    scene.AddSystem(collisionSys);
    scene.AddSystem(renderSys);

    EntityID player = scene.NewEntity();
    Transform3D *playerTransform = scene.Assign<Transform3D>(player);
    CameraComponent *playerCamera = scene.Assign<CameraComponent>(player);
    playerCamera->fov = 90.0f;
    playerCamera->far = 5000.0f;
    playerCamera->near = 0.15f;
    playerTransform->position.x = WINDOW_HEIGHT / -2.0f;
    playerTransform->position.y = WINDOW_WIDTH / 2.0f;
    playerTransform->rotation.z = -45.0f;

    for (u32 i = 0; i < NUM_BALLS; i++)
    {
        SpawnBall(scene, BALL_RADIUS, i % 10 == 0);
    }
}

void GameUpdateAndRender(Scene &scene, SDL_Window *window)
{
    scene.UpdateSystems();
}
