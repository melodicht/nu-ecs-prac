#pragma once 

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef uint32_t MeshID;
typedef uint32_t TextureID;

// Represents a vertex of a mesh (CPU->GPU)
struct Vertex
{
    glm::vec3 position;
    float uvX;
    glm::vec3 normal;
    float uvY;
};

// Represents the transformation data of the camera (CPU->GPU)
struct CameraData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 pos;
};

// Represents the transformation data of the objects in the scene (CPU->GPU)
struct ObjectData
{
    glm::mat4 model;
    glm::vec4 color;
};

// Represents the transformation data and lighting information of the scene
struct DirLightData
{
    glm::mat4 lightSpace;
    glm::vec3 lightDir;
    TextureID texture;
};

enum CullMode
{
    NONE,
    FRONT,
    BACK
};