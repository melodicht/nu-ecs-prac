#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 2 0
struct CameraData_0
{
    mat4x4 view_0;
    mat4x4 proj_0;
    vec3 pos_0;
};


#line 2
layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer_CameraData_0_1
{
    CameraData_0 _data;
};

#line 10
struct ObjectData_0
{
    mat4x4 model_0;
    vec4 color_0;
};

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer_ObjectData_0_2
{
    ObjectData_0 _data;
};

#line 17
struct Vertex_0
{
    vec3 pos_1;
    float uvX_0;
    vec3 normal_0;
    float uvY_0;
};

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer_Vertex_0_3
{
    Vertex_0 _data;
};

#line 17
struct VertPushConstants_natural_0
{
    BufferPointer_CameraData_0_1 camera_0;
    BufferPointer_ObjectData_0_2 objects_0;
    BufferPointer_Vertex_0_3 vertices_0;
};


#line 34
layout(push_constant)
layout(scalar) uniform block_VertPushConstants_natural_0
{
    BufferPointer_CameraData_0_1 camera_0;
    BufferPointer_ObjectData_0_2 objects_0;
    BufferPointer_Vertex_0_3 vertices_0;
}pcs_0;

#line 11760 1
layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer__S4_5
{
    mat4x4 _data;
};

#line 37 0
void main()
{

#line 37
    gl_Position = (((((((((vec4((pcs_0.vertices_0 + uint(gl_VertexIndex))._data.pos_1, 1.0)) * ((pcs_0.objects_0 + (uint(gl_BaseInstance) + uint(gl_InstanceIndex - gl_BaseInstance)))._data.model_0)))) * (((pcs_0.camera_0)._data.view_0))))) * (((pcs_0.camera_0)._data.proj_0))));

#line 37
    return;
}

