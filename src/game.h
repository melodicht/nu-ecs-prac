#pragma once

#include "game_platform.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
