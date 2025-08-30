#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// This represents constants/typedefs often used throughout program
// Really any compile time math concept

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
typedef float f32;
typedef double f64;

struct Transform3D
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale = glm::vec3(1);
};
