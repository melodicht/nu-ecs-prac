struct CameraData
{
    float4x4 view;
    float4x4 proj;
    float3 pos;
};

struct PushConstants
{
    CameraData *camera;
    float4x4 *objects;
    float4 *vertices;
};

[vk::push_constant]
PushConstants pcs;

struct VertexOutput
{
    float4 pos : SV_Position;
    float3 eyeRelPos : EyeRelPos;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexID: SV_VertexID, uint instanceID: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation)
{
    VertexOutput output;

    float4x4 model = pcs.objects[baseInstance + instanceID];

    float4 pos = pcs.vertices[vertexID];

    output.eyeRelPos = mul(model, pos).xyz - pcs.camera->pos;

    output.pos = mul(pcs.camera->proj, mul(pcs.camera->view, mul(model, pos)));
    return output;
};

[shader("fragment")]
float4 fragmentMain(float3 eyeRelPos: EyeRelPos) : SV_Target
{
    float3 dFdx = ddx(eyeRelPos);
    float3 dFdy = ddy(eyeRelPos);
    float3 normal = normalize(cross(dFdx, dFdy));
    constexpr float3 lightDir = normalize(float3(0.75, -1.0, -0.5));

    float brightness = 0.5 + (0.5 * dot(normal, -lightDir));

    return float4(0.2, 1.0, 0.4, 1.0) * brightness;
};



