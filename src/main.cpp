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

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include "renderer_gl.cpp"
#include "draw_utils.cpp"
#include "ecs.cpp"

#include "ecsprac.h"
#include "systems.cpp"
#include "ecsprac.cpp"

#include "platform_metrics.cpp"

int main() {
  srand (static_cast <unsigned> (time(0)));
	u64 cpuTimerFreq = EstimateCPUTimerFreq();

	SDL_Window* window = NULL;
	SDL_GLContext context;
	SDL_Surface* screenSurface = NULL;
	SDL_Renderer* renderer = NULL;
	if(!SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS))
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_LoadLibrary(NULL);

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("SDL Tutorial", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if(window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	renderer = SDL_CreateRenderer(window, NULL);

	if (renderer == NULL) 
	{
		printf("Rednerer could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		printf("OpenGL context could not be created: %s\n!", SDL_GetError());
		return 1;
	}

	if (!gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress))
	{
		printf("Failed to initialize GLAD!");
		return 1;
	}

	InitRenderer();

	screenSurface = SDL_GetWindowSurface(window);
	// Creates a renderer on the first driver that supports our 0 flags
	
	Scene scene;
	GameInitialize(scene, renderer);

	u64 lastCounter = ReadCPUTimer();


	SDL_Event e;
	bool playing = true;
	while(playing)
	{
		while(SDL_PollEvent(&e))
		{

			if(e.type == SDL_EVENT_QUIT)
			{
				playing = false;
			}
		/*

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
		*/

		}

		GameUpdateAndRender(scene, window, renderer);

			u64 endCounter = ReadCPUTimer();

			u64 counterElapsed = endCounter - lastCounter;
			f32 msPerFrame = 1000.0f * (f32)counterElapsed/(f32)cpuTimerFreq;
			f32 fps = (f32)cpuTimerFreq/(f32)counterElapsed;
			printf("%.02f ms/frame (FPS: %.02f)\n", msPerFrame, fps);
			lastCounter = endCounter;
		
		SDL_RenderPresent(renderer);
		//SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyWindow( window );
	SDL_Quit();
  return 0;
}
