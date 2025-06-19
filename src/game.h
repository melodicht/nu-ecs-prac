#pragma once

// NOTE(marvin): Game platform

#include "meta_definitions.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

// NOTE(marvin): Game

#include "renderer/render_types.h"

uint32_t cuboidMesh;
uint32_t trapMesh;
uint32_t pyraMesh;
uint32_t prismMesh;

struct Rigidbody
{
    float v_x;
    float v_y;
};

struct CircleCollider
{
    float radius;
};

struct GravityComponent
{
    float strength; // acceleration
};

struct ColorComponent
{
    f32 r;
    f32 g;
    f32 b;
};

struct MeshComponent
{
    MeshID mesh;
};

struct CameraComponent
{
    float fov;
    // NOTE(marvin): Why the `xx`? `near` and `far`
    // alone don't work... a dependency must have
    // used the name or something.
    float nearxx;
    float farxx;
};

struct FlyingMovement
{
    float moveSpeed;
    float turnSpeed;
};

struct Plane
{
    f32 width;
    f32 length;
};

struct DirLight
{
    glm::vec3 diffuse;
    glm::vec3 specular;

    TextureID shadowID = -1;
    CameraID cameraID = -1;
};

struct SpotLight
{
    glm::vec3 diffuse;
    glm::vec3 specular;

    TextureID shadowID = -1;
    CameraID cameraID = -1;

    f32 innerCone;
    f32 outerCone;
    f32 range;
};

struct PointLight
{
    glm::vec3 diffuse;
    glm::vec3 specular;

    TextureID shadowID = -1;
    CameraID cameraID = -1;

    f32 constant;
    f32 linear;
    f32 quadratic;

    f32 maxRange;
};
// Centered on the transform position
