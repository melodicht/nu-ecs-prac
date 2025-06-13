#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <random>

#include "skl_logger.h"

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

#if SKL_ENABLED_EDITOR
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#endif

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

#if EMSCRIPTEN
#include <emscripten/html5.h>
#endif
struct AppInformation
{
    SDL_Window *window;
    Scene& scene;
    SDL_Event& e;
    bool playing;
    u64 now;
    u64 last;

    AppInformation(SDL_Window *setWindow, Scene& setScene, SDL_Event& setE, bool setPlaying, u64 setNow, u64 setLast) :
        window(setWindow),
        scene(setScene),
        e(setE),
        playing(setPlaying),
        now(setNow),
        last(setLast)
    { }
};

void updateLoop(void* appInfo) {
    AppInformation* info = (AppInformation* )appInfo;
    info->last = info->now;
    info->now = SDL_GetPerformanceCounter();

    f32 deltaTime = (f32)((info->now - info->last) / (f32)SDL_GetPerformanceFrequency());

    while (SDL_PollEvent(&info->e))
    {
        // Cut off Imgui until we actually implement a base renderer for WGPU
        #if SKL_ENABLED_EDITOR
        ImGui_ImplSDL3_ProcessEvent(&info->e);
        #endif
        switch (info->e.type)
        {
            case SDL_EVENT_QUIT:
                info->playing = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (info->e.key.key == SDLK_ESCAPE)
                {
                    info->playing = false;
                }
                keysDown[SDL_GetKeyName(info->e.key.key)] = true;
                break;
            case SDL_EVENT_KEY_UP:
                keysDown[SDL_GetKeyName(info->e.key.key)] = false;
                break;
        }
    }

    SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);

    SDL_GetWindowSize(info->window, &windowWidth, &windowHeight);

    // Cut off Imgui until we actually implement a base renderer for WGPU
    #if SKL_ENABLED_EDITOR
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    #endif

    GameUpdateAndRender(info->scene, info->window, deltaTime);

    mouseDeltaX = 0;
    mouseDeltaY = 0;

    f32 msPerFrame =  1000.0f * deltaTime;
    f32 fps = 1 / deltaTime;
    // printf("%.02f ms/frame (FPS: %.02f)\n", msPerFrame, fps);
    return;
}


#include <filesystem>

int main()
{
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;
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

    #if SKL_ENABLED_EDITOR
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplSDL3_InitForOther(window);
    #endif

    SDL_SetWindowRelativeMouseMode(window, true);

    RenderInitDescriptor initDesc {
        .window = window,
        .startWidth = WINDOW_WIDTH,
        .startHeight = WINDOW_HEIGHT
    };
    InitRenderer(initDesc);

    Scene scene;
    GameInitialize(scene);

    SDL_Event e;
    bool playing = true;

    u64 now = SDL_GetPerformanceCounter();
    u64 last = 0;
    AppInformation app = AppInformation(window, scene, e, playing, now, last);
    #if EMSCRIPTEN
    emscripten_set_main_loop_arg(
        [](void* userData) {
            updateLoop(userData);
        }, 
        (void*)&app, 
        0, true
    );
    #else
    while (app.playing)
    {
        updateLoop(&app);
    }
    #endif
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
