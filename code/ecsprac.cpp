#include "ecsprac.h"

// Spawns a ball with the given radius in the given scene.
inline EntityID
SpawnBall(Scene &scene, float radius)
{
	EntityID ball = scene.NewEntity();
	TransformComponent* pBallTransform = scene.Assign<TransformComponent>(ball);
	Rigidbody* pBallRb = scene.Assign<Rigidbody>(ball);
	CircleCollider* pBallCC = scene.Assign<CircleCollider>(ball);
	ColorComponent* pColorComponent = scene.Assign<ColorComponent>(ball);

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
GameInitialize(Scene &scene)
{
  for (u32 i = 0; i < NUM_BALLS; i++)
  {
		SpawnBall(scene, BALL_RADIUS);
  }
}

// Scans for collision of a single component
// and edits trajectory of ball otherwise
void scanCollision(CircleCollider* checkCollider, Rigidbody* accessRigid, TransformComponent* accessTransform, Scene& accessScene){
  for (EntityID ent : SceneView<TransformComponent, Rigidbody, CircleCollider>(accessScene))
  {
    TransformComponent* t = accessScene.Get<TransformComponent>(ent);
    Rigidbody* rb = accessScene.Get<Rigidbody>(ent);
    CircleCollider* cc = accessScene.Get<CircleCollider>(ent);
    if(rb != accessRigid){
      double diffX = t->x_pos - accessTransform->x_pos;
      double diffY = t->y_pos - accessTransform->y_pos;
      double distance = sqrt(diffX * diffX + diffY * diffY);
      if(distance < cc->radius + checkCollider->radius){
        double normX = diffX/distance;
        double normY = diffY/distance;
        double thisSpeedMag = -sqrt(accessRigid->v_x * accessRigid->v_x + accessRigid->v_y * accessRigid->v_y);
        accessRigid->v_x = normX * thisSpeedMag;
        accessRigid->v_y = normY * thisSpeedMag;
        double speedMag = sqrt(rb->v_x * rb->v_x + rb->v_y * rb->v_y);
        rb->v_x = normX * speedMag;
        rb->v_y = normY * speedMag;
      }
    }
  }
}

void GameUpdateAndRender(Scene &scene, SDL_Window* window, SDL_Renderer* renderer)
{
  SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // BLACK
	SDL_RenderFillRect(renderer, NULL);

	// Gravity
	for (EntityID ent : SceneView<Rigidbody, GravityComponent>(scene))
	{
		Rigidbody* rb = scene.Get<Rigidbody>(ent);
		GravityComponent* gc = scene.Get<GravityComponent>(ent);

		rb->v_y += gc->strength;
	}

	// Forward movement, collision, rendering
  for (EntityID ent : SceneView<TransformComponent, Rigidbody, CircleCollider, ColorComponent>(scene))
  {
		TransformComponent* t = scene.Get<TransformComponent>(ent);
		Rigidbody* rb = scene.Get<Rigidbody>(ent);
		CircleCollider* cc = scene.Get<CircleCollider>(ent);
		ColorComponent* colc = scene.Get<ColorComponent>(ent);
		float radius = cc->radius;

		// Not framerate independent for simpler collision logic.
		t->x_pos += rb->v_x;
		t->y_pos += rb->v_y;

		// Collision check x-axis
		if ((t->x_pos - radius) < 0 || (t->x_pos + radius) > WINDOW_WIDTH)
		{
			rb->v_x *= -1;
		}

		// Collision check y-axis
		if ((t->y_pos - radius) < 0 || (t->y_pos + radius) > WINDOW_HEIGHT)
		{
			rb->v_y *= -1;
		}

		scanCollision(cc, rb, t, scene);
		
		float colorBrighteningFactor = std::min(sqrt((rb->v_x * rb->v_x) + (rb->v_y * rb->v_y)), 1.0f);
		u32 r = (u32)((t->x_pos / WINDOW_WIDTH) * 255);
		u32 g = (u32)((t->y_pos / WINDOW_HEIGHT) * 255);
		u32 b = (u32)(colc->b * colorBrighteningFactor) % 256;

		DrawFilledCircle(renderer, t->x_pos, t->y_pos, radius, r, g, b);
  }
}
