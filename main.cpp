#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>
#include <bitset>
#include <queue>

// for the ball game we make with the ECS
#define BALL_RADIUS 20
#define NUM_BALLS 1000

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

// Loads Renderer
#include "SfmlRenderWindow.hpp"

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
  SfmlRenderWindow window = SfmlRenderWindow::SfmlRenderWindowBuilder().build();
  srand (static_cast <unsigned> (time(0)));
  
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
    
    pBallRender->renderColor = Color(255,255,255);
    pBallRender->transform = pBallTransform;
    pBallTransform->x_pos = RandInBetween(radius, WINDOW_WIDTH - radius);
    pBallTransform->y_pos = RandInBetween(radius, WINDOW_HEIGHT - radius);
    pBallRb->v_x = RandInBetween(100, 150);
    pBallRb->v_y = RandInBetween(100, 150);
    pBallTransform->radius = radius;

  }
  ECSManager::start();
  TimeManager::init();

  // run the program as long as the window is open
  while (window.isActive())
  {
    window.pollEvent();

    ECSManager::update();
    TimeManager::update();

    window.renderScene(scene);
  }
}

