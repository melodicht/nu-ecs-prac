#include <algorithm>
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
#include "imgui.cpp"

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
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

			if (event.type == sf::Event::MouseMoved)
			{
				sf::Vector2i mousePos = sf::Mouse::getPosition();
				uistate.mousex = mousePos.x;
				uistate.mousey = mousePos.y;
			}

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				uistate.mousedown = 1;
			}
			else
			{
				uistate.mousedown = 0;
			}
    }

		sf::CircleShape circleTmp(50.f);
		circleTmp.setFillColor(sf::Color(0, 250, 250 * uistate.mousedown));
		circleTmp.setPosition(uistate.mousex, uistate.mousey);
		window.draw(circleTmp);

		// TODO(marv): Going to comment out game code for now just to get UI working!
		// GameUpdateAndRender(scene, window);
    
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
