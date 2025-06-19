#if defined(__unix__) || defined(__unix) || defined(unix) ||    \
    (defined(__APPLE__) && defined(__MACH__))
    #define PLATFORM_UNIX
#elif defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#endif

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

#include "game.h"

#if SKL_ENABLED_EDITOR
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#endif

#include "asset_types.h"
#include "renderer/render_backend.h"
#include "math/math_utils.cpp"

#include "main.h"

#include "platform_metrics.cpp"

#if EMSCRIPTEN
#include <emscripten/html5.h>
#endif

#define GAME_CODE_FILE_NAME "game-module"

global_variable std::unordered_map<std::string, bool> keysDown;
global_variable f32 mouseDeltaX = 0;
global_variable f32 mouseDeltaY = 0;

#ifdef PLATFORM_UNIX
local void *UnixLoadSymbol(void *handle, const char *symbol, b32 *failed)
{
    dlerror();
    void *symbolAddress = dlsym(handle, symbol);
    char *error = dlerror();
    if (error)
    {
        LOG_ERROR("Unable to find symbol in loaded game code:");
        LOG_ERROR(symbol);
        *failed = true;
    }
    else
    {
        *failed = false;
    }
    return symbolAddress;
}
#endif

local void SDLLoadGameCode(SDLGameCode *gameCode)
{

    const char *gameCodeFilePath;
#ifdef PLATFORM_WINDOWS
    gameCodeFilePath = GAME_CODE_FILE_NAME ".dll";
#else
    gameCodeFilePath = GAME_CODE_FILE_NAME ".so";
#endif
    SDL_SharedObject *sharedObjectHandle = SDL_LoadObject(gameCodeFilePath);
    if (!sharedObjectHandle)
    {
        LOG_ERROR("Game code loading failed.");
    }

    gameCode->gameInitialize = (game_initialize_t *)SDL_LoadFunction(sharedObjectHandle, "GameInitialize");
    gameCode->gameUpdateAndRender = (game_update_and_render_t *)SDL_LoadFunction(sharedObjectHandle, "GameUpdateAndRender");
    if (!gameCode->gameInitialize || !gameCode->gameUpdateAndRender)
    {
        LOG_ERROR("Unable to load symbols from game shared object.");
        gameCode->gameInitialize = 0;
        gameCode->gameUpdateAndRender = 0;
    }

    // NOTE(marvin):
    // These are platform specific implementations because I didn't know SDL provided
    // a cross-platform one. Just keeping this here just in case we end up making our
    // own platform layer.
    #if 0
    return;
    //if PLATFORM_UNIX
    // TODO(marvin): Test this shit.
    void *mainProgramHandle = dlopen(GAME_CODE_FILE_NAME ".so", RTLD_NOW);
    if (!mainProgramHandle)
    {
        LOG_ERROR("Game code loading failed.");
    }

    b32 gameInitializeLoadFailed;
    b32 gameUpdateAndRenderLoadFailed;
    gameCode->gameInitialize = (game_initialize_t *) UnixLoadSymbol(mainProgramHandle, "GameInitialize", &gameInitializeLoadFailed);
    gameCode->gameUpdateAndRender = (game_update_and_render_t *) UnixLoadSymbol(mainProgramHandle, "GameUpdateAndRender", &gameUpdateAndRenderLoadFailed);
    if (gameInitializeLoadFailed || gameUpdateAndRenderLoadFailed)
    {
        gameCode->gameInitialize = 0;
        gameCode->gameUpdateAndRender = 0;
    }
    //elif defined PLATFORM_WINDOWS
    HMODULE moduleHandle = LoadLibraryA(GAME_CODE_FILE_NAME ".dll");
    if (!moduleHandle)
    {
        LOG_ERROR("Game code loading failed.");
    }
    gameCode->gameInitialize = (game_initialize_t *) GetProcAddress(moduleHandle, "GameInitialize");
    gameCode->gameUpdateAndRender = (game_update_and_render_t *) GetProcAddress(moduleHandle, "GameUpdateAndRender");
    if (!gameCode->gameInitialize || !gameCode->gameUpdateAndRender)
    {
        LOG_ERROR("Unable to load symbol(s) of game code.");
        gameCode->gameInitialize = 0;
        gameCode->gameUpdateAndRender = 0;
    }
    //else
    LOG_ERROR("Unrecognized platform. Unable to load game code.");
    //endif
    #endif
}

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

    s32 windowWidth = WINDOW_WIDTH;
    s32 windowHeight = WINDOW_HEIGHT;
    SDL_GetWindowSize(info->window, &windowWidth, &windowHeight);

    // Cut off Imgui until we actually implement a base renderer for WGPU
    #if SKL_ENABLED_EDITOR
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    #endif

    SDLGameCode gameCode = info->gameCode;
    GameInput gameInput;
    gameInput.mouseDeltaX = mouseDeltaX;
    gameInput.mouseDeltaY = mouseDeltaY;
    gameInput.keysDown = keysDown;
    gameCode.gameUpdateAndRender(info->scene, gameInput, deltaTime);

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

    InitRenderer(window, WINDOW_WIDTH, WINDOW_HEIGHT);

    SDLGameCode gameCode = {};
    SDLLoadGameCode(&gameCode);

    Scene scene;
    gameCode.gameInitialize(scene);

    SDL_Event e;
    bool playing = true;

    u64 now = SDL_GetPerformanceCounter();
    u64 last = 0;
    AppInformation app = AppInformation(window, gameCode, scene, e, playing, now, last);
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
