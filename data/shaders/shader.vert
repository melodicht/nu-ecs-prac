#version 460 core
layout (location = 0) in vec3 aPos;
layout(std430, binding = 0) buffer objectBuffer
{
    mat4 models[];
};

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * models[gl_InstanceID + gl_BaseInstance] * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}