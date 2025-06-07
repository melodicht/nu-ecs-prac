// Represents camera that player sees through
struct Camera {
    viewMat : mat4x4<f32>,
    projMat : mat4x4<f32>,
    pos: vec3<f32>,
}

// Represents the data that differentiates each instance of the same mesh
struct ObjData {
    transform : mat4x4<f32>,
    color : vec4<f32>,
}

@binding(0) @group(0) var<uniform> camera : Camera;

@binding(1) @group(0) var<storage> objStore : array<ObjData>; 


struct VertexIn {
    @location(0) position: vec3<f32>,
    @location(1) uvX : f32,
    @location(2) normal : vec3<f32>,
    @location(3) uvY : f32,
    @builtin(instance_index) instance: u32, // Represents which instance within objStore to pull data from
}

struct VertexOut {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) camToVertRelPos: vec4<f32>,
}

// Collects translation
fn getTranslate(in : mat4x4<f32>) -> vec3<f32> {
    return vec3<f32>(in[3][0], in[3][1], in[3][2]);
}

@vertex
fn vtxMain(in : VertexIn) -> VertexOut {
  var out : VertexOut;

  var worldPos = objStore[in.instance].transform * vec4<f32>(in.position,1);

  out.position = camera.projMat * camera.viewMat * worldPos;
  out.color = objStore[in.instance].color;

  return out;
}

@fragment
fn fsMain(in : VertexOut) -> @location(0) vec4<f32>  {
    return in.color;
}