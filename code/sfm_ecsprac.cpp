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

#include "ecs.cpp"
#include "ecsprac.cpp"

#include "platform_metrics.cpp"

int main() {
  srand (static_cast <unsigned> (time(0)));
	u64 cpuTimerFreq = EstimateCPUTimerFreq();
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "BALLS!");
  Scene scene;

  GameInitialize(scene);

	u64 lastCounter = ReadCPUTimer();
  
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

		u64 endCounter = ReadCPUTimer();

		u64 counterElapsed = endCounter - lastCounter;
		f32 msPerFrame = 1000.0f * (f32)counterElapsed/(f32)cpuTimerFreq;
		f32 fps = (f32)cpuTimerFreq/(f32)counterElapsed;
		printf("%.02f ms/frame (FPS: %.02f)\n", msPerFrame, fps);
		lastCounter = endCounter;
  }

  return 0;
}