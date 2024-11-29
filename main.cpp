#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>
#include <bitset>

// for the ball game we make with the ECS
#define BALL_RADIUS 20
#define NUM_BALLS 5

// For rendering
#include "ComponentPool.hpp"
#include "ECSconsts.hpp"
#include "Scene.hpp"
#include "SceneView.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Represents a point in a 2D space as a Component
class TransformComponent {
    public:
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

int ecsDemo() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "ECS Tester");


  Scene scene;

  EntityID newEnt = scene.NewEntity();
  TransformComponent* transComp = scene.Assign<TransformComponent>(newEnt);

  sf::Clock deltaClock;

  transComp->x_pos = 100;
  transComp->y_pos = 100;

  // these can be turned into components, currently they're here for proof of concept
  // If you're seeing this, it means I haven't turned them into components yet
  // because some architecture questions need to be considered
  float ballAcceleration = 98;
  float ballMass = 5;

  // run the program as long as the window is open
  while (window.isOpen())
  {
    // Restarts the clock to get the change in time between frames
    // used for framerate-independent physics calculations
    sf::Time dt = deltaClock.restart();
    // check all the window's events that were triggered since the last iteration of the loop
    sf::Event event;

    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    } 
    // clear the window with black color
    window.clear(sf::Color::Black);

    // draw everything here...
    sf::CircleShape shape(50);

    // set the shape color to green
    shape.setFillColor(sf::Color(100, 250, 50));

    // set the absolute position of the shape to the transform component
    shape.setPosition(transComp->x_pos, transComp->y_pos);

    window.draw(shape);



    // Modifies the transform component
    // Setting this after the frame has been drawn is relatively arbitrary at the moment
    // As we develop the architecture of the system more, this will change.
    // e.g if we want to have physics calculations happen before a frame is drawn.
    transComp->y_pos += ballAcceleration * dt.asSeconds(); 
                                          // we multiply by dt.asSeconds() to make it framerate-independent
                                          // e.g if two seconds passed between frames, we multiply the distance
                                          // the ball should've travelled in a frame, times 2.
    ballAcceleration += ballMass * ballAcceleration * dt.asSeconds();
    printf("%f\n", transComp->y_pos);


    // end the current frame
    window.display();

  }

  return 0;
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
// Generates a random float in the inclusive range of the two given
// floats.
float RandInBetween(float LO, float HI)
{
  // From https://stackoverflow.com/questions/686353/random-float-number-generation
  return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
}

int main() {
  srand (static_cast <unsigned> (time(0)));
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "BALLS!");
  Scene scene;

  // Instantiate all the balls.
  for (u32 i = 0; i < NUM_BALLS; i++)
    {
      EntityID ball = scene.NewEntity();
      TransformComponent* pBallTransform = scene.Assign<TransformComponent>(ball);
      Rigidbody* pBallRb = scene.Assign<Rigidbody>(ball);
      CircleCollider* pBallCC = scene.Assign<CircleCollider>(ball);
      float radius = BALL_RADIUS;

      pBallTransform->x_pos = RandInBetween(radius, WINDOW_WIDTH - radius);
      pBallTransform->y_pos = RandInBetween(radius, WINDOW_HEIGHT - radius);
      pBallRb->v_x = RandInBetween(0.1, 0.2);
      pBallRb->v_y = RandInBetween(0.1, 0.2);
      pBallCC->radius = radius;
    }
  
  // run the program as long as the window is open
  while (window.isOpen())
  {
    // check all the window's events that were triggered since the last iteration of the loop
    sf::Event event;

    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    } 
    // clear the window with black color
    window.clear(sf::Color::Black);

    for (EntityID ent : SceneView<TransformComponent, Rigidbody, CircleCollider>(scene))
      {
        TransformComponent* t = scene.Get<TransformComponent>(ent);
        Rigidbody* rb = scene.Get<Rigidbody>(ent);
        CircleCollider* cc = scene.Get<CircleCollider>(ent);
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
        // Render to window
        sf::CircleShape shape(radius);
        shape.setFillColor(sf::Color(100, 250, 50));

        shape.setPosition(t->x_pos, t->y_pos);

        window.draw(shape);
      }
    
    // end the current frame
    window.display();

  }
}

