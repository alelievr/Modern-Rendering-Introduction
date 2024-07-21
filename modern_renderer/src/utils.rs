use std::slice;
use std::sync::Mutex;
use std::time::SystemTime;
use std::{fs, path::Path};
use caldera::prelude::*;
use std::collections::HashMap;
use log::debug;
use log::error;
use spark::{vk, Device};
use lazy_static::lazy_static;

const FORCE_COMPILATION_FIRST_RUN: bool = false;

lazy_static! {
    static ref COMPILED_SHADERS_MAP : Mutex<HashMap<String, SystemTime>> = {
        Mutex::new(HashMap::new())
    };
}

fn compile_hlsl_if_needed(shader_name_hlsl: &str, entry_point: &str, target_profile: &str) -> Result<String, &'static str>
{
    if let Err(e) = fs::create_dir_all("./spv/bin") {
        error!("Failed to create directory 'artifacts': {}", e);
        return Err("Failed to create directory 'artifacts'");
    }
    
    let path = Path::new(shader_name_hlsl);
    let path_name = path.file_name().unwrap().to_str().unwrap();
    let shader_name = format!("{0}_{1}.spv", path_name, target_profile);
    let spv_path = format!("./spv/bin/{0}", shader_name);

    // Read HLSL source file:
    let source = fs::read_to_string(shader_name_hlsl).unwrap();

    // Compare the last modified date of the spv and hlsl files, if the spv file is newer, skip compilation
    let last_modified_hlsl = fs::metadata(shader_name_hlsl).unwrap().modified().unwrap();
    let mut last_modified_spv : SystemTime = SystemTime::UNIX_EPOCH;
    let spv_meta = fs::metadata(&spv_path);
    if  spv_meta.is_ok()
    {
        last_modified_spv = spv_meta.unwrap().modified().unwrap();
    }

    // Check if this shader was already compiled during this session.
    let compiled_now = COMPILED_SHADERS_MAP.lock().unwrap().contains_key(shader_name_hlsl);
    // We force the compilation of shader every time when the program starts
    // if last_modified_hlsl > last_modified_spv || (!compiled_now && FORCE_COMPILATION_FIRST_RUN)
    // {
    //     COMPILED_SHADERS_MAP.lock().unwrap().insert(shader_name_hlsl.to_string(), last_modified_hlsl);
    //     log::info!("Recompiling shader: {}", shader_name_hlsl);
    //     let result = hassle_rs::utils::compile_hlsl(
    //         path_name,
    //         source.as_str(),
    //         entry_point,
    //         target_profile,
    //         &[/*"-Zi", "-Od", */"-I assets/shaders", "-spirv", "-fvk-use-scalar-layout", "-fspv-extension=SPV_EXT_descriptor_indexing"],
    //         &[],
    //     );
    //     match result {
    //         Ok(bytecode) => {
    //             debug!("Shader {} compiled successfully", shader_name_hlsl);
    //             // Write compiled shader to disk:
    //             fs::write(&spv_path, &bytecode).unwrap(); 
    //         }
    //         Err(error) => {
    //             log::error!("Cannot compile shader: {}", &error);
    //             return Err("Cannot compile shader");
    //         }
    //     }
    // }
    Ok(shader_name)
}

pub fn dispatch_compute(
    device: &Device,
    pipeline_cache: &PipelineCache,
    cmd: vk::CommandBuffer,
    shader_name_hlsl: &str,
    entry_point: &str,
    constants: &[SpecializationConstant],
    descriptor_set: DescriptorSet,
    grid_size: UVec2)
{
    let shader_name = compile_hlsl_if_needed(shader_name_hlsl, entry_point, "cs_6_0");
    if shader_name.is_ok()
    {
        dispatch_helper(device, pipeline_cache, cmd, shader_name.unwrap().as_str(), constants, descriptor_set, grid_size);
    }
}

pub fn render_primitives(
    device: &Device,
    pipeline_cache: &PipelineCache,
    cmd: vk::CommandBuffer,
    state: &GraphicsPipelineState,
    vertex_shader_name_hlsl: &str,
    fragment_shader_name_hlsl: &str,
    descriptor_set: DescriptorSet,
    vertex_count: u32,
    instance_count: u32
)
{
    let vertex: Result<String, &str> = compile_hlsl_if_needed(vertex_shader_name_hlsl, "vert", "vs_6_0");
    let fragment: Result<String, &str> = compile_hlsl_if_needed(fragment_shader_name_hlsl, "frag", "ps_6_0");
    if vertex.is_err() || fragment.is_err()
    {
        return;
    }

    let pipeline_layout = pipeline_cache.get_pipeline_layout(slice::from_ref(&descriptor_set.layout));
    let pipeline = pipeline_cache.get_graphics(
        VertexShaderDesc::standard(vertex.unwrap().as_str()),
        fragment.unwrap().as_str(),
        pipeline_layout,
        state,
    );
    unsafe {
        device.cmd_bind_pipeline(cmd, vk::PipelineBindPoint::GRAPHICS, pipeline);
        device.cmd_bind_descriptor_sets(
            cmd,
            vk::PipelineBindPoint::GRAPHICS,
            pipeline_layout,
            0,
            slice::from_ref(&descriptor_set.set),
            &[],
        );
        device.cmd_draw(cmd, vertex_count, instance_count, 0, 0);
    }
}