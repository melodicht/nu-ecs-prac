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

// A dynamically sized store for uniform
struct ObjArray {
    objs : array<ObjData>
}

@binding(0) @group(0) var<uniform> camera : Camera;

@binding(1) @group(0) var<uniform> objStore : ObjArray; 

@binding(2) @group(0) var<uniform> baseIdx : u32;     // The index of the first instance of the mesh within objStore

@binding(3) @group(0) var<uniform> instanceIdx : u32; // The index of the specified instance relative to baseIdx


struct VertexIn {
    @location(0) position: vec3<f32>,
}

struct VertexOut {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec3<f32>
}

// Collects translation
fn getTranslate(in : mat4x4<f32>) -> vec3<f32> {
    return vec3<f32>(in[3][0], in[3][1], in[3][2]);
}

@vertex
fn vtxMain(in : VertexIn) -> VertexOut {
  var out : VertexOut;
  out.position = camera.projMat * camera.viewMat * objStore.objs[baseIdx + instanceIdx].transform * vec4<f32>(in.position, 1);
  out.color = objStore.objs[baseIdx + instanceIdx].color;

  return out;
}

@frag
fn fsMain(in : VertexOut) -> @location(0) vec4<f32>  {
    return vec4<f32>(in.color, 1);
}