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

struct LightCascade
{
    float4x4 lightSpace;
    float maxDepth;
}

// Represents a single vertex of a mesh
struct Vertex
{
    float3 pos;
    float uvX;
    float3 normal;
    float uvY;
};

struct PushConstants
{
    CameraData *camera;
    ObjectData *objects;
    Vertex *vertices;
    float3 lightDir;
    uint shadowID;
    LightCascade* cascades;
    uint cascadeCount;
}


[vk::push_constant]
PushConstants pcs;

// Represents a vertex that has been processed by the vertex shader
struct CoarseVertex
{
    float4 color;
    float4 worldPos;
    float3 normal;
    float3 eyeRelPos;
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

    float3 normal = normalize(mul(normMat, vert.normal));

    CoarseVertex vertData;
    vertData.eyeRelPos = worldPos.xyz - pcs.camera->pos;
    vertData.worldPos = worldPos;
    vertData.normal = normal;
    vertData.color = object.color;

    output.pos = mul(pcs.camera->proj, mul(pcs.camera->view, worldPos));
    output.vertData = vertData;

    return output;
};


[[vk::binding(0,0)]]
Sampler2DArrayShadow[] shadowArrays;

[shader("fragment")]
float4 fragmentMain(CoarseVertex vertData : CoarseVertex) : SV_Target
{
    float4 viewPos = mul(pcs.camera->view, vertData.worldPos);

    int cascade = -1;
    for (int i = 0; i < pcs.cascadeCount; i++)
    {
        if (viewPos.z < pcs.cascades[i].maxDepth)
        {
            cascade = i;
            break;
        }
    }
    if (cascade == -1)
    {
        cascade = pcs.cascadeCount;
    }

    float4 lightRelPos = mul(pcs.cascades[cascade].lightSpace, vertData.worldPos + float4(vertData.normal * 10, 0.0));

    float3 normal = vertData.normal;

    float3 lightPosNorm = (lightRelPos.xyz / lightRelPos.w);
    float3 lightPosScaled = float3(lightPosNorm.xy * 0.5 + 0.5, lightPosNorm.z);

    float unshadowed = 0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            unshadowed += shadowArrays[pcs.shadowID].SampleCmp(float3(lightPosScaled.xy, cascade),
                                                               lightPosScaled.z, int2(x, y)) / 9;
        }
    }

    float ambient = 0.1;
    float diffuse = max(dot(normal, -pcs.lightDir), 0.0);
    float3 viewDir = normalize(-vertData.eyeRelPos);
    float3 halfDir = normalize(viewDir - pcs.lightDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 32.0);

    float brightness = ambient + (unshadowed * (diffuse + specular));

    return vertData.color * brightness;
};