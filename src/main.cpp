#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <random>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200

#include "math/math_consts.h"

#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "renderer/render_backend.h"

#include "asset_utils.cpp"

#include "math/math_utils.cpp"

int windowWidth = WINDOW_WIDTH;
int windowHeight = WINDOW_HEIGHT;

std::unordered_map<std::string, bool> keysDown;
f32 mouseDeltaX = 0;
f32 mouseDeltaY = 0;

#include "ecs.cpp"

#include "game.h"
#include "systems.cpp"
#include "game.cpp"

#include "platform_metrics.cpp"

int main()
{
    srand(static_cast<unsigned>(time(0)));

    SDL_Window *window = NULL;
    SDL_Surface *screenSurface = NULL;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Untitled Engine", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | GetRenderWindowFlags());
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplSDL3_InitForOther(window);

    SDL_SetWindowRelativeMouseMode(window, true);

    InitRenderer(window, WINDOW_WIDTH, WINDOW_HEIGHT);

    Scene scene;
    GameInitialize(scene);

    SDL_Event e;
    bool playing = true;

    u64 now = SDL_GetPerformanceCounter();
    u64 last = 0;
    while (playing)
    {
        last = now;
        now = SDL_GetPerformanceCounter();

        f32 deltaTime = (f32)((now - last) / (f32)SDL_GetPerformanceFrequency());

        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);

            switch (e.type)
            {
                case SDL_EVENT_QUIT:
                    playing = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (e.key.key == SDLK_ESCAPE)
                    {
                        playing = false;
                    }
                    keysDown[SDL_GetKeyName(e.key.key)] = true;
                    break;
                case SDL_EVENT_KEY_UP:
                    keysDown[SDL_GetKeyName(e.key.key)] = false;
                    break;
            }
        }

        SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);

        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        GameUpdateAndRender(scene, window, deltaTime);

        mouseDeltaX = 0;
        mouseDeltaY = 0;

        f32 msPerFrame =  1000.0f * deltaTime;
        f32 fps = 1 / deltaTime;
        //printf("%.02f ms/frame (FPS: %.02f)\n", msPerFrame, fps);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
