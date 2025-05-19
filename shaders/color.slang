// Represents transformation data for the camera
struct CameraData
{
    float4x4 view;
    float4x4 proj;
    float3 pos;
};

// Represents transformation and material data for the objects in the scene
struct ObjectData
{
    float4x4 model;
    float4 color;
};

// Represents a single vertex of a mesh
struct Vertex
{
    float3 pos;
    float uvX;
    float3 normal;
    float uvY;
};

// Represents the memory locations of the camera, object, and vertex buffers
struct PushConstants
{
    CameraData *camera;
    ObjectData *objects;
    Vertex *vertices;
};

[vk::push_constant]
PushConstants pcs;

// Represents a vertex that has been processed by the vertex shader
struct CoarseVertex
{
    float3 eyeRelPos;
    float3 normal;
    float4 color;
};

// Represents a processed vertex along with its screenspace position
struct VertexOutput
{
    float4 pos : SV_Position;
    CoarseVertex vertData : CoarseVertex;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexID : SV_VertexID,
    uint instanceID : SV_InstanceID, uint baseInstance : SV_StartInstanceLocation)
{
    ObjectData object = pcs.objects[baseInstance + instanceID];

    float4x4 model = object.model;
    Vertex vert = pcs.vertices[vertexID];
    float4 pos = float4(vert.pos, 1.0);
    float4 worldPos = mul(model, pos);

    VertexOutput output;
    float3x3 normMat = float3x3(
                cross(model[1].xyz, model[2].xyz),
                cross(model[2].xyz, model[0].xyz),
                cross(model[0].xyz, model[1].xyz));

    CoarseVertex vertData;
    vertData.color = object.color;
    vertData.eyeRelPos = worldPos.xyz - pcs.camera->pos;
    vertData.normal = normalize(mul(normMat, vert.normal));

    output.pos = mul(pcs.camera->proj, mul(pcs.camera->view, worldPos));
    output.vertData = vertData;

    return output;
};

[shader("fragment")]
float4 fragmentMain(CoarseVertex vertData : CoarseVertex) : SV_Target
{
    float3 normal = vertData.normal;
    constexpr float3 lightDir = normalize(float3(1.0, 0.75, -0.5));

    float ambient = 0.1;
    float diffuse = max(dot(normal, -lightDir), 0.0);
    float3 viewDir = normalize(-vertData.eyeRelPos);
    float3 halfDir = normalize(viewDir - lightDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 32.0);

    float brightness = ambient + diffuse + specular;

    return vertData.color * brightness;
};