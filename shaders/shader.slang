struct CameraData
{
    float4x4 view;
    float4x4 proj;
    float3 pos;
};

struct ObjectData
{
    float4x4 model;
    float4 color;
};
struct PushConstants
{
    CameraData *camera;
    ObjectData *objects;
    float4 *vertices;
};

[vk::push_constant]
PushConstants pcs;

struct VertexOutput
{
    float4 pos : SV_Position;
    float4 color : VertColor;
    float3 eyeRelPos : EyeRelPos;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexID: SV_VertexID, uint instanceID: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation)
{
    VertexOutput output;
    ObjectData object = pcs.objects[baseInstance + instanceID];

    float4x4 model = object.model;

    float4 pos = pcs.vertices[vertexID];

    output.pos = mul(pcs.camera->proj, mul(pcs.camera->view, mul(model, pos)));
    output.color = object.color;
    output.eyeRelPos = mul(model, pos).xyz - pcs.camera->pos;

    return output;
};

[shader("fragment")]
float4 fragmentMain(float4 color : VertColor, float3 eyeRelPos: EyeRelPos) : SV_Target
{
    float3 dFdx = ddx(eyeRelPos);
    float3 dFdy = ddy(eyeRelPos);
    float3 normal = normalize(cross(dFdx, dFdy));
    constexpr float3 lightDir = normalize(float3(0.75, -1.0, -0.5));

    float ambient = 0.1;
    float diffuse = max(dot(normal, -lightDir), 0.0);
    float3 viewDir = normalize(-eyeRelPos);
    float3 halfDir = normalize(viewDir - lightDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 32.0);

    float brightness = ambient + diffuse + specular;

    return color * brightness;
};



