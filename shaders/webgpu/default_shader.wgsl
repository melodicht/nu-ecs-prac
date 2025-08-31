// TODO: Better explore alternatives in alignment, maybe we could merge in different variables to reduce waste due to alignment

// Represents fixed length color pass data
struct ColorPassFixedData {
    // Camera Data
    combinedMat : mat4x4<f32>,
    viewMat : mat4x4<f32>,
    projMat : mat4x4<f32>,
    pos: vec3<f32>,
    // Light Data
    dirLightAmount: u32
}

// Represents the data that differentiates each instance of the same mesh
struct ObjData {
    transform : mat4x4<f32>,
    normMat : mat4x4<f32>,
    color : vec4<f32>
}

// All dynamic directional lights must have a set amount of cascades.
// TODO: Hopefully we can create a way of modifying this at CPU side with some shader metaprogramming
const dynamicShadowedDirLightCascadeAmount : u32 = 1;

// TODO: Create specified ambient lighting

// Represents a single directional light with shadows and a potential to change pos/dir over time.
struct DynamicShadowedDirLight {
    lightSpaces : array<mat4x4<f32>, dynamicShadowedDirLightCascadeAmount>,
    diffuse : vec3<f32>,
    diffuseIntensity : f32,
    specular : vec3<f32>,
    specularIntensity : f32,
    direction : vec3<f32>,
    padding : f32, // Fill with useful stuff later
}

@binding(0) @group(0) var<uniform> fixedData : ColorPassFixedData;

@binding(1) @group(0) var<storage, read> objStore : array<ObjData>; 

@binding(2) @group(0) var<storage, read> dynamicShadowedDirLightStore : array<DynamicShadowedDirLight>;

@binding(3) @group(0) var dynamicShadowedDirLightStoreStore : texture_depth_2d_array;

@binding(4) @group(0) var shadowMapSampler : sampler_comparison;


struct VertexIn {
    @location(0) position: vec3<f32>,
    @location(1) uvX : f32,
    @location(2) normal : vec3<f32>,
    @location(3) uvY : f32,
    @builtin(instance_index) instance: u32, // Represents which instance within objStore to pull data from
}

// Collects translation from a mat4x4 
fn getTranslate(in : mat4x4<f32>) -> vec3<f32> {
    return vec3<f32>(in[3][0], in[3][1], in[3][2]);
}

// Default pipeline for color pass 
struct ColorPassVertexOut {
    @builtin(position) position: vec4<f32>,
    @location(0) fragToCamPos: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) normal: vec3<f32>,
    @location(3) worldPos: vec4<f32>
}

@vertex
fn vtxMain(in : VertexIn) -> ColorPassVertexOut {
  var out : ColorPassVertexOut;

  var worldPos = objStore[in.instance].transform * vec4<f32>(in.position,1);

  out.worldPos = worldPos;
  out.position = fixedData.combinedMat * worldPos;
  out.color = objStore[in.instance].color.xyz;
  out.fragToCamPos = fixedData.pos - worldPos.xyz;
  var nMat = objStore[in.instance].normMat;
  out.normal = normalize(mat3x3(nMat[0].xyz, nMat[1].xyz, nMat[2].xyz) * in.normal);

  return out;
}

// TODO: Implement blinn-phong instead of just phong
@fragment
fn fsMain(in : ColorPassVertexOut) -> @location(0) vec4<f32>  {
    // TODO: Set ambient lighting to be specified

    // Sets ambient lighting
    var ambientIntensity : f32 = 0.25;
    var ambient : vec3<f32> = in.color * ambientIntensity;

    // Sets diffuse lighting
    var diffuse : vec3<f32> = vec3<f32>(0, 0, 0);

    var viewDir : vec3<f32> = normalize(in.fragToCamPos);
    var specular : vec3<f32> = vec3<f32>(0, 0, 0);
    for (var dirIter : u32 = 0 ; dirIter < fixedData.dirLightAmount ; dirIter++) {
        var lightsUncovered : f32 = 1;
        for (var lightIter : u32 = 0 ; lightIter < dynamicShadowedDirLightCascadeAmount ; lightIter++) {
            // Checks if location has been covered by light
            var lightSpacePosition : vec4<f32> = dynamicShadowedDirLightStore[dirIter].lightSpaces[lightIter] * in.worldPos; 
            lightSpacePosition = lightSpacePosition / lightSpacePosition.w;
            var texturePosition: vec3<f32> = vec3<f32>((lightSpacePosition.x * 0.5) + 0.5, (lightSpacePosition.y * -0.5) + 0.5, lightSpacePosition.z);
            var bias: f32 = max(0.05 * (1.0 - dot(in.normal, dynamicShadowedDirLightStore[dirIter].direction)), 0.005); 
            lightsUncovered = lightsUncovered * textureSampleCompare(dynamicShadowedDirLightStoreStore, shadowMapSampler, texturePosition.xy, dirIter, texturePosition.z - bias);
        }
        var diffuseIntensity : f32 = lightsUncovered * max(dot(in.normal, dynamicShadowedDirLightStore[dirIter].direction), 0.0);
        diffuse += diffuseIntensity * dynamicShadowedDirLightStore[dirIter].diffuse;

        var specularIntensity : f32 = lightsUncovered * pow(max(dot(viewDir, reflect(-dynamicShadowedDirLightStore[dirIter].direction, in.normal)), 0.0), 32);
        specular += specularIntensity * dynamicShadowedDirLightStore[dirIter].specular;
    }

    return vec4<f32>(((diffuse + specular) * in.color) + ambient,1);
}