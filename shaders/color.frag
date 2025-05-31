#version 450
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_texture_shadow_lod : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 91 0
struct FragPushConstants_natural_0
{
    vec3 lightDir_0;
    uint shadowID_0;
};


#line 95
layout(push_constant)
layout(scalar) uniform block_FragPushConstants_natural_0
{
    layout(offset = 24) vec3 lightDir_0;
    uint shadowID_0;
}fpcs_0;

#line 98
layout(binding = 0)
uniform sampler2DShadow  shadowMaps_0[];


#line 2148 1
layout(location = 0)
out vec4 entryPointParam_fragmentMain_0;


#line 2148
layout(location = 0)
in vec3 vertData_eyeRelPos_0;


#line 2148
layout(location = 1)
in vec4 vertData_lightRelPos_0;


#line 2148
layout(location = 2)
in vec3 vertData_normal_0;


#line 2148
layout(location = 3)
in vec4 vertData_color_0;


#line 101 0
void main()
{


    vec3 lightPosNorm_0 = vertData_lightRelPos_0.xyz / vertData_lightRelPos_0.w;
    vec3 _S1 = vec3(lightPosNorm_0.xy * 0.5 + 0.5, lightPosNorm_0.z);

#line 114
    uint _S2 = fpcs_0.shadowID_0;

#line 114
    vec2 _S3 = _S1.xy;

#line 114
    float _S4 = _S1.z;

#line 114
    const ivec2 _S5 = ivec2(-1, -1);

#line 114
    ;

#line 114
    vec3 _S6 = vec3(_S3, _S4);

#line 114
    float _S7 = (textureOffset((shadowMaps_0[_S2]), (_S6), (_S5)));

#line 114
    float _S8 = _S7 / 9.0;

#line 114
    uint _S9 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S10 = ivec2(-1, 0);

#line 114
    ;

#line 114
    float _S11 = (textureOffset((shadowMaps_0[_S9]), (_S6), (_S10)));

#line 114
    float unshadowed_0 = _S8 + _S11 / 9.0;

#line 114
    uint _S12 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S13 = ivec2(-1, 1);

#line 114
    ;

#line 114
    float _S14 = (textureOffset((shadowMaps_0[_S12]), (_S6), (_S13)));

#line 114
    float unshadowed_1 = unshadowed_0 + _S14 / 9.0;

#line 114
    uint _S15 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S16 = ivec2(0, -1);

#line 114
    ;

#line 114
    float _S17 = (textureOffset((shadowMaps_0[_S15]), (_S6), (_S16)));

#line 114
    float unshadowed_2 = unshadowed_1 + _S17 / 9.0;

#line 114
    uint _S18 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S19 = ivec2(0, 0);

#line 114
    ;

#line 114
    float _S20 = (textureOffset((shadowMaps_0[_S18]), (_S6), (_S19)));

#line 114
    float unshadowed_3 = unshadowed_2 + _S20 / 9.0;

#line 114
    uint _S21 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S22 = ivec2(0, 1);

#line 114
    ;

#line 114
    float _S23 = (textureOffset((shadowMaps_0[_S21]), (_S6), (_S22)));

#line 114
    float unshadowed_4 = unshadowed_3 + _S23 / 9.0;

#line 114
    uint _S24 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S25 = ivec2(1, -1);

#line 114
    ;

#line 114
    float _S26 = (textureOffset((shadowMaps_0[_S24]), (_S6), (_S25)));

#line 114
    float unshadowed_5 = unshadowed_4 + _S26 / 9.0;

#line 114
    uint _S27 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S28 = ivec2(1, 0);

#line 114
    ;

#line 114
    float _S29 = (textureOffset((shadowMaps_0[_S27]), (_S6), (_S28)));

#line 114
    float unshadowed_6 = unshadowed_5 + _S29 / 9.0;

#line 114
    uint _S30 = fpcs_0.shadowID_0;

#line 114
    const ivec2 _S31 = ivec2(1, 1);

#line 114
    ;

#line 114
    float _S32 = (textureOffset((shadowMaps_0[_S30]), (_S6), (_S31)));

#line 114
    entryPointParam_fragmentMain_0 = vertData_color_0 * (0.10000000149011612 + (unshadowed_6 + _S32 / 9.0) * (max(dot(vertData_normal_0, - fpcs_0.lightDir_0), 0.0) + pow(max(dot(vertData_normal_0, normalize(normalize(- vertData_eyeRelPos_0) - fpcs_0.lightDir_0)), 0.0), 32.0)));

#line 114
    return;
}

