#pragma once

#include "renderer/render_types.h"
#include "ecs.h"
#include "SDL3/SDL.h"

void GameInitialize(Scene &scene);

void GameUpdateAndRender(Scene &scene, SDL_Window *window, SDL_Renderer *renderer);

#define BALL_RADIUS 5
#define NUM_BALLS 1000

#define GRAVITY_MIN 0.25
#define GRAVITY_MAX 0.5

u32 cuboidMesh;
u32 trapMesh;
u32 pyraMesh;
u32 prismMesh;

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

    LightID lightID = -1;
};

struct SpotLight
{
    glm::vec3 diffuse;
    glm::vec3 specular;

    LightID lightID = -1;

    f32 innerCone;
    f32 outerCone;
    f32 range;
};

struct PointLight
{
    glm::vec3 diffuse;
    glm::vec3 specular;

    LightID lightID = -1;

    f32 constant;
    f32 linear;
    f32 quadratic;

    f32 maxRange;
};
// Centered on the transform position
