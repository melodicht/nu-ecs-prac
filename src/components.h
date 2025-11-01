// DO NOT INCLUDE!!
// Include component_def.h instead for the component struct definitions

#ifndef COMP
#error Do not include this file directly, include component_def.h instead for the component struct definitions.
#endif

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "renderer/render_types.h"
#include "math/skl_math_consts.h"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/CharacterVirtual.h>

// Define the game's components here

COMP(MeshComponent)
{
    FIELD(MeshID, mesh, -1);
    FIELD(glm::vec3, color, glm::vec3{0.5f});
    LOCAL_FIELD(bool, dirty, true);
};

COMP(PlayerCharacter)
{
    LOCAL_FIELD(JPH::CharacterVirtual*, characterVirtual, nullptr);
};

COMP(CameraComponent)
{
    FIELD(float, fov, 90);
    FIELD(float, nearPlane, 0.01);
    FIELD(float, farPlane, 1000);
};

COMP(FlyingMovement)
{
    FIELD(float, moveSpeed, 5);
    FIELD(float, turnSpeed, 0.1);
};

COMP(Plane)
{
    FIELD(float, width, 1);
    FIELD(float, length, 1);
};

COMP(DirLight)
{
    FIELD(glm::vec3, diffuse, glm::vec3{1});
    FIELD(glm::vec3, specular, glm::vec3{1});
    LOCAL_FIELD(LightID, lightID, -1);
};

COMP(SpotLight)
{
    FIELD(glm::vec3, diffuse, glm::vec3{1});
    FIELD(glm::vec3, specular, glm::vec3{1});
    LOCAL_FIELD(LightID, lightID, -1);

    FIELD(float, innerCone, 30);
    FIELD(float, outerCone, 45);
    FIELD(float, range, 100);
};

COMP(PointLight)
{
    FIELD(glm::vec3, diffuse, glm::vec3{1});
    FIELD(glm::vec3, specular, glm::vec3{1});
    LOCAL_FIELD(LightID, lightID, -1);

    FIELD(float, constant, 1);
    FIELD(float, linear, 0.25);
    FIELD(float, quadratic, 0.2);

    FIELD(float, maxRange, 20);
};