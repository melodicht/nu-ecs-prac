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
    mat4x4 lightSpace_0;
};


#line 2
layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer_CameraData_0_1
{
    CameraData_0 _data;
};

#line 11
struct ObjectData_0
{
    mat4x4 model_0;
    vec4 color_0;
};

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer_ObjectData_0_2
{
    ObjectData_0 _data;
};

#line 18
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

#line 18
struct VertPushConstants_natural_0
{
    BufferPointer_CameraData_0_1 camera_0;
    BufferPointer_ObjectData_0_2 objects_0;
    BufferPointer_Vertex_0_3 vertices_0;
};


#line 35
layout(push_constant)
layout(scalar) uniform block_VertPushConstants_natural_0
{
    BufferPointer_CameraData_0_1 camera_0;
    BufferPointer_ObjectData_0_2 objects_0;
    BufferPointer_Vertex_0_3 vertices_0;
}vpcs_0;

#line 12015 1
layout(location = 0)
out vec3 entryPointParam_vertexMain_vertData_eyeRelPos_0;


#line 12015
layout(location = 1)
out vec4 entryPointParam_vertexMain_vertData_lightRelPos_0;


#line 12015
layout(location = 2)
out vec3 entryPointParam_vertexMain_vertData_normal_0;


#line 12015
layout(location = 3)
out vec4 entryPointParam_vertexMain_vertData_color_0;


#line 39 0
struct CoarseVertex_0
{
    vec3 eyeRelPos_0;
    vec4 lightRelPos_0;
    vec3 normal_1;
    vec4 color_1;
};

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer__S4_5
{
    vec3 _data;
};
layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer BufferPointer__S6_7
{
    mat4x4 _data;
};

#line 48
struct VertexOutput_0
{
    vec4 pos_2;
    CoarseVertex_0 vertData_0;
};



void main()
{

    ObjectData_0 object_0 = (vpcs_0.objects_0 + (uint(gl_BaseInstance) + uint(gl_InstanceIndex - gl_BaseInstance)))._data;

#line 64
    vec4 worldPos_0 = (((vec4((vpcs_0.vertices_0 + uint(gl_VertexIndex))._data.pos_1, 1.0)) * ((vpcs_0.objects_0 + (uint(gl_BaseInstance) + uint(gl_InstanceIndex - gl_BaseInstance)))._data.model_0)));



    vec3 _S8 = (vpcs_0.objects_0 + (uint(gl_BaseInstance) + uint(gl_InstanceIndex - gl_BaseInstance)))._data.model_0[1].xyz;

#line 68
    vec3 _S9 = (vpcs_0.objects_0 + (uint(gl_BaseInstance) + uint(gl_InstanceIndex - gl_BaseInstance)))._data.model_0[2].xyz;
    vec3 _S10 = (vpcs_0.objects_0 + (uint(gl_BaseInstance) + uint(gl_InstanceIndex - gl_BaseInstance)))._data.model_0[0].xyz;


    vec3 normal_2 = normalize(((((vpcs_0.vertices_0 + uint(gl_VertexIndex))._data.normal_0) * (mat3x3(cross(_S8, _S9), cross(_S9, _S10), cross(_S10, _S8))))));
    vec4 shadowPos_0 = worldPos_0 + vec4(normal_2 * 16.0, 0.0);

    CoarseVertex_0 vertData_1;
    vertData_1.eyeRelPos_0 = worldPos_0.xyz - ((vpcs_0.camera_0)._data.pos_0);
    vertData_1.lightRelPos_0 = (((shadowPos_0) * (((vpcs_0.camera_0)._data.lightSpace_0))));
    vertData_1.normal_1 = normal_2;
    vertData_1.color_1 = object_0.color_0;

#line 66
    VertexOutput_0 output_0;

#line 81
    output_0.pos_2 = ((((((worldPos_0) * (((vpcs_0.camera_0)._data.view_0))))) * (((vpcs_0.camera_0)._data.proj_0))));
    output_0.vertData_0 = vertData_1;

    VertexOutput_0 _S11 = output_0;

#line 84
    gl_Position = output_0.pos_2;

#line 84
    entryPointParam_vertexMain_vertData_eyeRelPos_0 = _S11.vertData_0.eyeRelPos_0;

#line 84
    entryPointParam_vertexMain_vertData_lightRelPos_0 = _S11.vertData_0.lightRelPos_0;

#line 84
    entryPointParam_vertexMain_vertData_normal_0 = _S11.vertData_0.normal_1;

#line 84
    entryPointParam_vertexMain_vertData_color_0 = _S11.vertData_0.color_1;

#line 84
    return;
}

