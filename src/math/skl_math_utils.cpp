#include "skl_math_utils.h"

#include <random>

glm::mat4 GetRotationMatrix(Transform3D *transform)
{
    glm::quat aroundX = glm::angleAxis(glm::radians(transform->rotation.x), glm::vec3(1.0, 0.0, 0.0));
    glm::quat aroundY = glm::angleAxis(glm::radians(transform->rotation.y), glm::vec3(0.0, 1.0, 0.0));
    glm::quat aroundZ = glm::angleAxis(glm::radians(transform->rotation.z), glm::vec3(0.0, 0.0, 1.0));
    return glm::mat4_cast(aroundZ * aroundY * aroundX);
}

glm::mat4 GetTransformMatrix(Transform3D *transform)
{
    return glm::scale(glm::translate(glm::mat4(1.0f), transform->position) * GetRotationMatrix(transform), transform->scale);
}

glm::vec3 GetForwardVector(const glm::mat4x4& rotMat)
{
    return rotMat * glm::vec4(1.0, 0.0, 0.0, 1.0);
}

glm::vec3 GetForwardVector(Transform3D *transform)
{
    return GetForwardVector(GetRotationMatrix(transform));
}

glm::vec3 GetRightVector(const glm::mat4x4& rotMat)
{
    return rotMat * glm::vec4(0.0, 1.0, 0.0, 1.0);
}

glm::vec3 GetRightVector(Transform3D *transform)
{
    return GetRightVector(GetRotationMatrix(transform));
}

glm::vec3 GetUpVector(const glm::mat4x4& rotMat)
{
    return rotMat * glm::vec4(0.0, 0.0, 1.0, 1.0);
}

glm::vec3 GetUpVector(Transform3D *transform)
{
    return GetUpVector(GetRotationMatrix(transform));
}

glm::mat4 GetViewMatrix(Transform3D *transform)
{
    glm::mat4 view = glm::mat4(0);
    glm::vec3 right = GetRightVector(transform);
    glm::vec3 up = GetUpVector(transform);
    glm::vec3 forward = GetForwardVector(transform);
    view[0][0] = right.x;
    view[1][0] = right.y;
    view[2][0] = right.z;
    view[3][0] = -glm::dot(right, transform->position);
    view[0][1] = up.x;
    view[1][1] = up.y;
    view[2][1] = up.z;
    view[3][1] = -glm::dot(up, transform->position);
    view[0][2] = forward.x;
    view[1][2] = forward.y;
    view[2][2] = forward.z;
    view[3][2] = -glm::dot(forward, transform->position);
    view[3][3] = 1;

    return view;
}

// Generates a random float in the inclusive range of the two given
// floats.
f32 RandInBetween(f32 LO, f32 HI)
{
    // From https://stackoverflow.com/questions/686353/random-float-number-generation
    return LO + static_cast<f32>(rand()) / (static_cast<f32>(RAND_MAX / (HI - LO)));
}

u32 RandInt(u32 min, u32 max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<u32> distribution(min, max);
    return distribution(gen);
}

glm::vec3 GetArbitraryOrthogonal(const glm::vec3& vec) {
  if (std::abs(vec.x) < std::abs(vec.y) && std::abs(vec.x) < std::abs(vec.z)) {
    return glm::normalize(glm::cross(vec, glm::vec3(1, 0, 0)));
  } else if (abs(vec.y) < abs(vec.z)) {
    return glm::normalize(glm::cross(vec, glm::vec3(0, 1, 0)));
  } else {
    return glm::normalize(glm::cross(vec, glm::vec3(0, 0, 1)));
  }
}

glm::mat4x4 GetMatrixSpace(const glm::vec3& forward, const glm::vec3& up, const glm::vec3& right) {
  return glm::mat4x4(
    glm::vec4(right, 0.0f),
    glm::vec4(up, 0.0f),
    glm::vec4(forward, 0.0f),
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
  );
}

std::vector<glm::vec4> getFrustumCorners(const glm::mat4& proj, const glm::mat4& view)
{
    glm::mat4 inverse = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (u32 x = 0; x < 2; ++x)
    {
        for (u32 y = 0; y < 2; ++y)
        {
            for (u32 z = 0; z < 2; ++z)
            {
                const glm::vec4 pt =
                        inverse * glm::vec4(
                                    2.0f * x - 1.0f,
                                    2.0f * y - 1.0f,
                                    z,
                                    1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}