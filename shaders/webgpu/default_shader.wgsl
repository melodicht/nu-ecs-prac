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

struct VertexData {
    position: vec3<f32>,
    uvX : f32,
    normal : vec3<f32>,
    uvY : f32,
}

@binding(0) @group(0) var<uniform> camera : Camera;

@binding(1) @group(0) var<storage> objStore : array<ObjData>; 

@binding(2) @group(0) var<storage> meshStore : array<VertexData>; 

struct VertexStageIn {
    @builtin(instance_index) instanceIdx: u32,
    @builtin(vertex_index) vertexIdx: u32,
}

struct VertexStageOut {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
}

// Collects translation
fn getTranslate(in : mat4x4<f32>) -> vec3<f32> {
    return vec3<f32>(in[3][0], in[3][1], in[3][2]);
}

@vertex
fn vtxMain(in : VertexStageIn) -> VertexStageOut {
  var out : VertexStageOut;
  out.position = camera.projMat * camera.viewMat * objStore[in.instanceIdx].transform * vec4(meshStore[in.vertexIdx].position, 1);
  out.color = objStore[in.instanceIdx].color;

  return out;
}

@fragment
fn fsMain(in : VertexStageOut) -> @location(0) vec4<f32>  {
    return in.color;
}