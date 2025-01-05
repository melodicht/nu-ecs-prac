#ifndef ECSPRAC_H

void
GameInitialize(Scene &scene);

void
GameUpdateAndRender(Scene &scene, sf::RenderWindow &window);

#define BALL_RADIUS 5
#define NUM_BALLS 250

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


#define ECSPRAC_H
#endif
