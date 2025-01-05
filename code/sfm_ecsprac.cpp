#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "utils.cpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define u8  uint8_t
#define u32 uint32_t
#define u64 uint64_t
typedef float f32;
typedef double f64;

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include "ecs.cpp"
#include "ecsprac.cpp"

#include "platform_metrics.cpp"

int main() {
  srand (static_cast <unsigned> (time(0)));
	sf::Clock beginClock;
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "BALLS!");
  Scene scene;

  GameInitialize(scene);

	u64 lastCounter = beginClock.getElapsedTime().asMicroseconds();
  
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

		GameUpdateAndRender(scene, window);
    
    window.display();

		u64 endCounter =  beginClock.getElapsedTime().asMicroseconds();

		u64 counterElapsed = endCounter - lastCounter;
		f32 msPerFrame = (counterElapsed / 1000.0f);
		f32 fps = (1/(msPerFrame / 1000.0f));
		printf("%.02f ms/frame (FPS: %.02f)\n", msPerFrame, fps);
		lastCounter = endCounter;
  }

  return 0;
}
