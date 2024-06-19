use std::panic;

use log::debug;
use rusty_d3d12::*;

pub fn create_compute_pso(
    root_signature: &RootSignature,
    compute_shader: Vec<u8>,
    device: &Device,
) -> PipelineState {
    let cs_bytecode = ShaderBytecode::new(&compute_shader);

    let pso_desc = ComputePipelineStateDesc::default()
        .with_root_signature(root_signature)
        .with_cs_bytecode(&cs_bytecode);

    let pso = device
        .create_compute_pipeline_state(&pso_desc)
        .expect("Cannot create PSO");
    pso
}

pub fn setup_root_signature(device: &Device) -> RootSignature {
    let mut feature_data =
        FeatureDataRootSignature::new(RootSignatureVersion::V1_1);
    if device
        .check_feature_support(Feature::RootSignature, &mut feature_data)
        .is_err()
    {
        feature_data.with_highest_version(RootSignatureVersion::V1_0);
        unimplemented!(
            "To support v1.0 root signature serialization we'd need to bring \
d3dx12.h as a dependency to have X12SerializeVersionedRootSignature"
        );
    }

    let ranges = vec![DescriptorRange::default()
        .with_range_type(DescriptorRangeType::Uav)
        .with_num_descriptors(1)
        .with_flags(DescriptorRangeFlags::DataStatic)];

    let descriptor_table =
        RootDescriptorTable::default().with_descriptor_ranges(&ranges);
    let root_parameters = vec![RootParameter::default()
        .new_descriptor_table(&descriptor_table)
        .with_shader_visibility(ShaderVisibility::All)];

    // let sampler_desc = StaticSamplerDesc::default()
    //     .with_filter(Filter::MinMagMipPoint)
    //     .with_address_u(TextureAddressMode::Border)
    //     .with_address_v(TextureAddressMode::Border)
    //     .with_address_w(TextureAddressMode::Border)
    //     .with_comparison_func(ComparisonFunc::Never)
    //     .with_border_color(StaticBorderColor::TransparentBlack)
    //     .with_shader_visibility(ShaderVisibility::Pixel);

    let root_signature_desc = VersionedRootSignatureDesc::default()
        .with_desc_1_1(
            &RootSignatureDesc::default()
                .with_parameters(&root_parameters)
                // .with_static_samplers(std::slice::from_ref(&sampler_desc))
                // .with_flags(RootSignatureFlags::AllowInputAssemblerInputLayout),
        );

    let (serialized_signature, serialization_result) =
        RootSignature::serialize_versioned(&root_signature_desc);
    assert!(
        serialization_result.is_ok(),
        "Result: {}",
        &serialization_result.err().unwrap()
    );

    let root_signature = device
        .create_root_signature(
            0,
            &ShaderBytecode::new(serialized_signature.get_buffer()),
        )
        .expect("Cannot create root signature");
    root_signature
}

pub fn compile_shader(
    name: &str,
    source: &str,
    entry_point: &str,
    shader_model: &str,
) -> Vec<u8> {
    let result = hassle_rs::utils::compile_hlsl(
        name,
        source,
        entry_point,
        shader_model,
        &["/Zi", "-Qembed_debug"],
        &[],
    );
    match result {
        Ok(bytecode) => {
            debug!("Shader {} compiled successfully", name);
            bytecode
        }
        Err(error) => {
            let format_error = error.replace("\n", "\n");
            panic!("Cannot compile shader {name}: {format_error}"); 
        }
    }
}
