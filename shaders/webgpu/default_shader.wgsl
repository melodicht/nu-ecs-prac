// Represents camera that player sees through
struct Camera {
    combinedMat : mat4x4<f32>,
    viewMat : mat4x4<f32>,
    projMat : mat4x4<f32>,
    pos: vec3<f32>,
}

// Represents the data that differentiates each instance of the same mesh
struct ObjData {
    transform : mat4x4<f32>,
    color : vec4<f32>,
}

// All dynamic directional lights must have a set amount of cascades.
// TODO: Hopefully we can create a way of modifying this at CPU side with some shader metaprogramming
const dynamicShadowedDirLightCascadeAmount : u32 = 4;

// Represents a single directional light with shadows and a potential to change pos/dir over time.
struct DynamicShadowedDirLight {
    lightSpaces : array<mat4x4<f32>, dynamicShadowedDirLightCascadeAmount>,
    direction : vec3<f32>,
    intensity : f32,
    color : vec3<f32>
}

@binding(0) @group(0) var<uniform> camera : Camera;

@binding(1) @group(0) var<storage, read> objStore : array<ObjData>; 

@binding(2) @group(0) var<storage, read> dynamicShadowedDirLightStore : array<DynamicShadowedDirLight>;

@binding(3) @group(0) var dynamicShadowedDirLightStoreStore: texture_2d_array<f32>;


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
    @location(0) color: vec4<f32>,
    @location(1) camToVertRelPos: vec4<f32>,
}

@vertex
fn vtxMain(in : VertexIn) -> ColorPassVertexOut {
  var out : ColorPassVertexOut;

  var worldPos = objStore[in.instance].transform * vec4<f32>(in.position,1);

  out.position = camera.combinedMat * worldPos;
  out.color = objStore[in.instance].color;

  return out;
}

@fragment
fn fsMain(in : ColorPassVertexOut) -> @location(0) vec4<f32>  {
    return in.color;
}