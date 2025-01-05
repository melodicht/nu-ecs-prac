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

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "ecs.cpp"
#include "ecsprac.cpp"

int main() {
  srand (static_cast <unsigned> (time(0)));
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "BALLS!");
  Scene scene;

  GameInitialize(scene);
  
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
  }

  return 0;
}
