#pragma once

#include "meta_definitions.h"

#include <bitset>   // For ECS
#include <unordered_map>
#include <string>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200

struct GameInput
{
  f32 mouseDeltaX;
  f32 mouseDeltaY;

  std::unordered_map<std::string, bool> keysDown;
};

// NOTE(marvin): Game platform only needs to know about scene, and only system needs to know about game input. Maybe separate out scene.h?
  
#include "ecs.h"

#define GAME_INITIALIZE(name) void name(Scene &scene)
typedef GAME_INITIALIZE(game_initialize_t);

#define GAME_UPDATE_AND_RENDER(name) void name(Scene &scene, GameInput &input, f32 deltaTime)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_t);
