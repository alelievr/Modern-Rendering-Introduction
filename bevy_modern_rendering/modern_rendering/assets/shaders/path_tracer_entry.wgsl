// @group(0) @binding(0) var input: texture_storage_2d<r32float, read>;

@group(0) @binding(0) var output: texture_storage_2d<rgba16float, write>;

struct Ray
{
    origin: vec3<f32>;
    direction: vec3<f32>;
};

struct Camera
{
    direction: vec3<f32>;
    position: vec3<f32>;
    near: f32;
    far: f32;
};

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) invocation_id: vec3<u32>, @builtin(num_workgroups) num_workgroups: vec3<u32>) {
    textureStore(output, invocation_id.xy, vec4<f32>(0));
}
