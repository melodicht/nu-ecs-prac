// Spawns a ball with the given radius in the given scene.
inline EntityID
SpawnBall(Scene &scene, float radius, bool hasGravity)
{
	EntityID ball = scene.NewEntity();
	TransformComponent* pBallTransform = scene.Assign<TransformComponent>(ball);
	Rigidbody* pBallRb = scene.Assign<Rigidbody>(ball);
	CircleCollider* pBallCC = scene.Assign<CircleCollider>(ball);
	ColorComponent* pColorComponent = scene.Assign<ColorComponent>(ball);

	if (hasGravity)
	{
		GravityComponent *pGravityComponent = scene.Assign<GravityComponent>(ball);
		pGravityComponent->strength = 2.0f;
	}

	pBallTransform->x_pos = RandInBetween(radius, WINDOW_WIDTH - radius);
	pBallTransform->y_pos = RandInBetween(radius, WINDOW_HEIGHT - radius);
	pBallRb->v_x = RandInBetween(0.1, 0.2);
	pBallRb->v_y = RandInBetween(0.1, 0.2);
	pBallCC->radius = radius;
	pColorComponent->r = (u32)RandInBetween(50, 256);
	pColorComponent->g = (u32)RandInBetween(50, 256);
	pColorComponent->b = (u32)RandInBetween(50, 256);
	return ball;
}

void
GameInitialize(Scene &scene, SDL_Renderer *renderer)
{
	GravitySystem *gravitySys = new GravitySystem();
	CollisionSystem *collisionSys = new CollisionSystem();
	RenderSystem *renderSys = new RenderSystem(renderer);
	scene.AddSystem(gravitySys);
	scene.AddSystem(collisionSys);
	scene.AddSystem(renderSys);
  for (u32 i = 0; i < NUM_BALLS; i++)
  {
		SpawnBall(scene, BALL_RADIUS, i % 10 == 0);
  }
}

void GameUpdateAndRender(Scene &scene, SDL_Window* window, SDL_Renderer* renderer)
{
  SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // BLACK
	SDL_RenderFillRect(renderer, NULL);

	scene.UpdateSystems();
}
