#pragma once 

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef int32_t MeshID;
typedef int32_t TextureID;
typedef int32_t LightID;

// Represents a vertex of a mesh (CPU->GPU)
struct Vertex
{
    glm::vec3 position;
    f32 uvX;
    glm::vec3 normal;
    f32 uvY;
};

// Represents the transformation data of the objects in the scene (CPU->GPU)
struct ObjectData
{
    glm::mat4 model;
    glm::vec4 color;
};

// Represents the transformation data of the camera (CPU->GPU)
struct CameraData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 pos;

    CameraData() :
        view(),
        proj(),
        pos()
    { }

    CameraData(glm::mat4 setView, glm::mat4 setProj, glm::vec3 setPos) :
        view(setView),
        proj(setProj),
        pos(setPos)
    { }
};

enum CullMode
{
    NONE,
    FRONT,
    BACK
};