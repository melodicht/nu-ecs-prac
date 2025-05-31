// Represents camera that player sees through
struct Camera {
    projMat : mat4x4<f32>,
    viewMat : mat4x4<f32>,
}

// Represents the data that differentiates each instance of the same mesh
struct ObjData {
    transform : mat4x4<f32>,
    color : vec3<f32>,
}

@binding(0) @group(0) var<uniform> camera : Camera;

@binding(1) @group(0) var<storage> objStore : array<ObjData>; 

@binding(2) @group(0) var<uniform> baseIdx : u32;     // The index of the first instance of the mesh within objStore

@binding(3) @group(0) var<uniform> instanceIdx : u32; // The index of the specified instance relative to baseIdx


struct VertexIn {
    @location(0) position: vec3<f32>,
    @location(1) uvX : f32,
    @location(2) normal : vec3<f32>,
    @location(3) uvY : f32,
}

struct VertexOut {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec3<f32>,
}

// Collects translation
fn getTranslate(in : mat4x4<f32>) -> vec3<f32> {
    return vec3<f32>(in[3][0], in[3][1], in[3][2]);
}

@vertex
fn vtxMain(in : VertexIn) -> VertexOut {
  var out : VertexOut;
  out.position = camera.projMat * camera.viewMat * objStore[baseIdx + instanceIdx].transform * vec4<f32>(in.position,1);
  out.color = objStore[baseIdx + instanceIdx].color;

  return out;
}

@fragment
fn fsMain(in : VertexOut) -> @location(0) vec4<f32>  {
    return vec4<f32>(in.color, 1);
}