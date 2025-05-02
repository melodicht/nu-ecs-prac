#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

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
#include <SDL3/SDL_vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <VkBootstrap.h>

#include "vma_no_warnings.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "math_utils.cpp"

int windowWidth = WINDOW_WIDTH;
int windowHeight = WINDOW_HEIGHT;

#include "renderer_vk.cpp"
#include "ecs.cpp"

#include "game.h"
#include "systems.cpp"
#include "game.cpp"

#include "platform_metrics.cpp"

int main()
{
    srand(static_cast<unsigned>(time(0)));
    u64 cpuTimerFreq = GetOSTimerFreq();

    if (volkInitialize() != VK_SUCCESS)
    {
        printf("Volk could not initialize!");
        return 1;
    }

    SDL_Window *window = NULL;
    SDL_Surface *screenSurface = NULL;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Untitled Engine", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    InitRenderer(window);

    Scene scene;
    GameInitialize(scene);

    u64 lastCounter = ReadOSTimer();


    SDL_Event e;
    bool playing = true;
    while (playing)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                playing = false;
            }
            if (e.type == SDL_EVENT_KEY_DOWN)
            {
                SDL_GetKeyName(e.key.key);
            }
        }

        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

        GameUpdateAndRender(scene, window);

        u64 endCounter = ReadOSTimer();

        u64 counterElapsed = endCounter - lastCounter;
        f32 msPerFrame = 1000.0f * (f32) counterElapsed / (f32) cpuTimerFreq;
        f32 fps = (f32) cpuTimerFreq / (f32) counterElapsed;
        printf("%.02f ms/frame (FPS: %.02f)\n", msPerFrame, fps);
        lastCounter = endCounter;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
