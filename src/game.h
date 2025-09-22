#pragma once

#include "game_platform.h"
#include "skl_thread_safe_primitives.h"
#include "debug.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// NOTE(marvin): Took Jolt includes from
// https://github.com/jrouwe/JoltPhysicsHelloWorld/blob/main/Source/HelloWorld.cpp

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "renderer/render_types.h"

MeshID cuboidMesh;
MeshID trapMesh;
MeshID pyraMesh;
MeshID prismMesh;

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

struct PlayerCharacter
{
    JPH::CharacterVirtual *characterVirtual;
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
