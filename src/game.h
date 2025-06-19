#pragma once

#include "meta_definitions.h"

#include "ecs.h"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200

#define GAME_INITIALIZE(name) void name(Scene &scene)
typedef GAME_INITIALIZE(game_initialize_t);

#define GAME_UPDATE_AND_RENDER(name) void name(Scene &scene, SDL_Window *window, f32 deltaTime)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_t);

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
    float near;
    float far;
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
