struct Transform3D
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale = glm::vec3(1);
};

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

glm::vec3 GetForwardVector(Transform3D *transform)
{
    return GetRotationMatrix(transform) * glm::vec4(1.0, 0.0, 0.0, 1.0);
}

glm::vec3 GetRightVector(Transform3D *transform)
{
    return GetRotationMatrix(transform) * glm::vec4(0.0, 1.0, 0.0, 1.0);
}

glm::vec3 GetUpVector(Transform3D *transform)
{
    return GetRotationMatrix(transform) * glm::vec4(0.0, 0.0, 1.0, 1.0);
}

glm::mat4 MakeViewMatrix(glm::vec3 forward, glm::vec3 right, glm::vec3 up, glm::vec3 position)
{
    glm::mat4 view = {};
    view[0] = {right.x, up.x, forward.x, 0};
    view[1] = {right.y, up.y, forward.y, 0};
    view[2] = {right.z, up.z, forward.z, 0};
    view[3] = {-glm::dot(right, position),
               -glm::dot(up, position),
               -glm::dot(forward, position),
               1};

    return view;
}

glm::mat4 GetViewMatrix(Transform3D *transform)
{
    glm::vec3 forward = GetForwardVector(transform);
    glm::vec3 right = GetRightVector(transform);
    glm::vec3 up = GetUpVector(transform);

    return MakeViewMatrix(forward, right, up, transform->position);
}

void GetPointViews(Transform3D *transform, glm::mat4 *views)
{
    glm::vec3 forward = GetForwardVector(transform);
    glm::vec3 right = GetRightVector(transform);
    glm::vec3 up = GetUpVector(transform);

    views[0] = MakeViewMatrix(forward, -up, right, transform->position);
    views[1] = MakeViewMatrix(-forward, up, right, transform->position);
    views[2] = MakeViewMatrix(right, forward, -up, transform->position);
    views[3] = MakeViewMatrix(-right, forward, up, transform->position);
    views[4] = MakeViewMatrix(up, forward, right, transform->position);
    views[5] = MakeViewMatrix(-up, -forward, right, transform->position);
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
