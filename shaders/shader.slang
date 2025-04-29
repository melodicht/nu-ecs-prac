struct CameraMats
{
    float4x4 view;
    float4x4 proj;
};

struct PushConstants
{
    CameraMats* camera;
    float4x4 *objects;
    float4 *vertices;
};

[vk::push_constant]
PushConstants pcs;

[shader("vertex")]
float4 vertexMain(uint vertexID: SV_VertexID, uint instanceID: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation) : SV_Position
{
    float4x4 model = pcs.objects[baseInstance + instanceID];
    return mul(pcs.camera->proj, mul(pcs.camera->view, mul(model, pcs.vertices[vertexID])));
};

[shader("fragment")]
float4 fragmentMain()
{
    return float4(0.2, 1.0, 0.4, 1.0);
};



