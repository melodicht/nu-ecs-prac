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

// Loads ECS System
#include "ComponentPool.hpp"
#include "ECSConsts.hpp"
#include "Scene.hpp"
#include "SceneView.hpp"

// Loads Components
#include "CircleCollider.hpp"
#include "CircleRenderer.hpp"
#include "Rigidbody.hpp"
#include "TransformComponent.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

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
  Scene& scene = ECSManager::getScene();

  // Instantiate all the balls.
  for (u32 i = 0; i < NUM_BALLS; i++)
  {
    EntityID ball = scene.NewEntity();
    CircleRenderer* pBallRender = scene.Assign<CircleRenderer>(ball);
    TransformComponent* pBallTransform = scene.Assign<TransformComponent>(ball);
    Rigidbody* pBallRb = scene.Assign<Rigidbody>(ball);
    CircleCollider* pBallCC = scene.Assign<CircleCollider>(ball);
    float radius = BALL_RADIUS;

    pBallRender->window = &window;
    pBallTransform->x_pos = RandInBetween(radius, WINDOW_WIDTH - radius);
    pBallTransform->y_pos = RandInBetween(radius, WINDOW_HEIGHT - radius);
    pBallRb->v_x = RandInBetween(0.1, 0.2);
    pBallRb->v_y = RandInBetween(0.1, 0.2);
    pBallTransform->radius = radius;

  }
  
  ECSManager::start();

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

    ECSManager::update();

    // end the current frame
    window.display();

  }
}

