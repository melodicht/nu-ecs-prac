#ifndef ECSPRAC_H

void
GameInitialize(Scene &scene);

void
GameUpdateAndRender(Scene &scene, sf::RenderWindow &window);

#define BALL_RADIUS 5
#define NUM_BALLS 1000

#define GRAVITY_MIN 0.25
#define GRAVITY_MAX 0.5

// Represents a point in a 2D space as a Component
struct TransformComponent {
  float x_pos;
  float y_pos;
};

struct Rigidbody {
  float v_x;
  float v_y;
};

struct CircleCollider {
  float radius;
};

struct GravityComponent {
	float strength;  // acceleration
};

struct ColorComponent {
	u32 r;
	u32 g;
	u32 b;
};


#define ECSPRAC_H
#endif
