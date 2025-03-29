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

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>

#include "draw_utils.cpp"
#include "ecs.cpp"
#include "ecsprac.cpp"
#include "platform_metrics.cpp"

int main() {
  srand (static_cast <unsigned> (time(0)));
	u64 cpuTimerFreq = EstimateCPUTimerFreq();

	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;
	SDL_Renderer* renderer = NULL;
	if(SDL_Init( SDL_INIT_VIDEO ) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		renderer = SDL_CreateRenderer(window, -1, 0);
		if(window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		} 
		else if (renderer == NULL) 
		{
			printf("Rednerer could not be created! SDL_Error: %s\n", SDL_GetError());
		} 
		else
		{
			screenSurface = SDL_GetWindowSurface(window);
			// Creates a renderer on the first driver that supports our 0 flags
			
			Scene scene;
			GameInitialize(scene);

			u64 lastCounter = ReadCPUTimer();


			SDL_Event e;
			bool playing = true;
			while(playing)
			{
				while(SDL_PollEvent(&e))
				{
					if(e.type == SDL_QUIT)
					{
						playing = false;
					}
					else if(e.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						screenSurface = SDL_GetWindowSurface(window);
					}
					else if(e.window.event == SDL_WINDOWEVENT_CLOSE)
					{
						playing = false;
					}
					else if(e.type == SDL_MOUSEMOTION)
					{
						SDL_MouseMotionEvent motionEvent = e.motion;
						// motionEvent.x
						// motionEvent.y
						// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent
						// Mapping from pixel to world like sfml?
					}
					else if (e.type == SDL_MOUSEBUTTONDOWN)
					{
						SDL_MouseButtonEvent buttonEvent = e.button;
						// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent
					}

					GameUpdateAndRender(scene, window, renderer);

					u64 endCounter = ReadCPUTimer();

					u64 counterElapsed = endCounter - lastCounter;
					f32 msPerFrame = 1000.0f * (f32)counterElapsed/(f32)cpuTimerFreq;
					f32 fps = (f32)cpuTimerFreq/(f32)counterElapsed;
					printf("%.02f ms/frame (FPS: %.02f)\n", msPerFrame, fps);
					lastCounter = endCounter;
				}
				
				SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
				SDL_UpdateWindowSurface(window);
			}
		}
	}

	SDL_DestroyWindow( window );
	SDL_Quit();
  return 0;
}
