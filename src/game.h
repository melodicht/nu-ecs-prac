#pragma once

void GameInitialize(Scene &scene);

void GameUpdateAndRender(Scene &scene, SDL_Window *window, SDL_Renderer *renderer);

#define BALL_RADIUS 5
#define NUM_BALLS 1000

#define GRAVITY_MIN 0.25
#define GRAVITY_MAX 0.5

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

struct DirectionalLight
{
    f32 strength;
};

struct SpotLight
{

};

struct PointLight
{
    
};
// Centered on the transform position
