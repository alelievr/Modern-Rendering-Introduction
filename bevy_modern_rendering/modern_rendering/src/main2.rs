use std::{borrow::Cow, ptr::null};
use bevy::{
    core_pipeline::core_3d::graph::Core3d, ecs::query::QueryItem, prelude::*, render::{
        extract_component::{
            ExtractComponent, ExtractComponentPlugin, UniformComponentPlugin,
        }, extract_resource::{ExtractResource, ExtractResourcePlugin}, render_asset::{RenderAssetUsages, RenderAssets}, render_graph::{
            self, NodeRunError, RenderGraph, RenderGraphApp, RenderGraphContext, RenderLabel, RenderSubGraph, ViewNode, ViewNodeRunner
        }, render_resource::{binding_types::texture_storage_2d, *}, renderer::{RenderContext, RenderDevice}, texture::GpuImage, view::ViewTarget, Render, RenderApp, RenderSet
    }
};
use bevy_mod_hlsl::{HLSLPlugin, HLSLRegistry};

const WINDOW_RESOLUTION: (u32, u32) = (1920, 1080);

#[derive(Debug, Hash, PartialEq, Eq, Clone, RenderSubGraph)]
pub struct ModernRenderer;

fn main()
{
    let mut app = App::new();

    // PanicHandlerPlugin,
    // LogPlugin::default(),
    // TaskPoolPlugin::default(),
    // TypeRegistrationPlugin,
    // FrameCountPlugin,
    // TimePlugin,
    // TransformPlugin,
    // HierarchyPlugin,
    // DiagnosticsPlugin,
    // InputPlugin,
    // WindowPlugin::default(),
    // AccessibilityPlugin,
    // ScenePlugin,
    // WinitPlugin::default(),
    // RenderPlugin::default(),
    // SpritePlugin,
    // TextPlugin,
    // UiPlugin,
    // PbrPlugin::default(),
    // AudioPlugin::default(),
    // GilrsPlugin,
    // GizmoPlugin,

    app.add_plugins((
        DefaultPlugins
        .set(WindowPlugin {
            primary_window: Some(Window {
                resolution: (
                    (WINDOW_RESOLUTION.0) as f32,
                    (WINDOW_RESOLUTION.1) as f32,
                )
                    .into(),
                // uncomment for unthrottled FPS
                // present_mode: bevy::window::PresentMode::AutoNoVsync,
                ..default()
            }),
            ..default()
        })
        .set(ImagePlugin::default_nearest()),
            HLSLPlugin,
            PathTracerPlugin,
        ))
        .add_systems(Startup, setup)
        .add_systems(Update, (rotate, update_settings))
        .add_systems(Update, switch_textures)
        .run();
}

struct PathTracerPlugin;

impl Plugin for PathTracerPlugin {
    fn build(&self, app: &mut App) {

        app.add_plugins(ExtractResourcePlugin::<BuiltinResources>::default());

        // Probably not needed, we'll use structured buffers instead for most use cases
        app.add_plugins((
            ExtractComponentPlugin::<PathTracerSettings>::default(),
            UniformComponentPlugin::<PathTracerSettings>::default(),
        ));

        let render_app = app.get_sub_app_mut(RenderApp).unwrap();

        render_app.add_systems(Render, prepare_bind_group.in_set(RenderSet::PrepareBindGroups));

        let mut render_graph = render_app.world_mut().resource_mut::<RenderGraph>();

        render_graph.add_node(PathTracerLabel, PathTracerNode::default());
        render_graph.add_node_edge(PathTracerLabel, bevy::render::graph::CameraDriverLabel);
    }

    fn finish(&self, app: &mut App) {
        let Some(render_app) = app.get_sub_app_mut(RenderApp) else {
            return;
        };

        render_app.init_resource::<PathTracerPipeline>();
    }
}

#[derive(Resource)]
struct PathTracerBindGroups([BindGroup; 1]);

#[derive(Debug, Hash, PartialEq, Eq, Clone, RenderLabel)]
struct PathTracerLabel;

fn prepare_bind_group(
    mut commands: Commands,
    pipeline: Res<PathTracerPipeline>,
    gpu_images: Res<RenderAssets<GpuImage>>,
    builtin_resources: Res<BuiltinResources>,
    render_device: Res<RenderDevice>,
) {
    let view = gpu_images.get(&builtin_resources.color_buffer).unwrap();
    let bind_group_0 = render_device.create_bind_group(
        None,
        &pipeline.layout,
        &BindGroupEntries::single(&view.texture_view),
    );
    commands.insert_resource(PathTracerBindGroups([bind_group_0]));
}

#[derive(Default)]
struct PathTracerNode;

fn switch_textures(images: Res<BuiltinResources>, mut displayed: Query<&mut Handle<Image>>) {
    let mut displayed: Mut<Handle<Image>> = displayed.single_mut();
    *displayed = images.color_buffer.clone_weak();
}

impl render_graph::Node for PathTracerNode {

    fn update(&mut self, world: &mut World) {
    }

    fn run(
        &self,
        _graph: &mut render_graph::RenderGraphContext,
        render_context: &mut RenderContext,
        world: &World,
    ) -> Result<(), render_graph::NodeRunError> {

        // TODO: move this outside of a render graph node
        // Check for requirements:
        if !render_context.render_device()
        .features()
        .contains(WgpuFeatures::SAMPLED_TEXTURE_AND_STORAGE_BUFFER_ARRAY_NON_UNIFORM_INDEXING | WgpuFeatures::PUSH_CONSTANTS)
        {
            error!(
                "Render device doesn't support feature \
                SAMPLED_TEXTURE_AND_STORAGE_BUFFER_ARRAY_NON_UNIFORM_INDEXING or PUSH_CONSTANTS."
            );
            return Ok(());
        }

        let path_tracer_pipeline = world.resource::<PathTracerPipeline>();

        let pipeline_cache = world.resource::<PipelineCache>();

        let Some(pipeline) = pipeline_cache.get_compute_pipeline(path_tracer_pipeline.pipeline_id)
        else {
            return Ok(());
        };

        // let settings_uniforms = world.resource::<ComponentUniforms<PathTracerSettings>>();
        // let Some(settings_binding) = settings_uniforms.uniforms().binding() else {
        //     return Ok(());
        // };

        let bind_groups = &world.resource::<PathTracerBindGroups>().0;

        let cmd = render_context.command_encoder();

        cmd.push_debug_group("Path Tracer Pass");
        {
            let mut pass = cmd.begin_compute_pass(&ComputePassDescriptor::default());
            pass.set_pipeline(pipeline);
            pass.set_bind_group(0, &bind_groups[0], &[]);
            pass.dispatch_workgroups(16, 16, 1);
        }
        cmd.pop_debug_group();

        println!("Path Tracer Node Run");

        // let mut render_pass = render_context.begin_tracked_render_pass(RenderPassDescriptor {
        //     label: Some("path_tracer_pass"),
        //     color_attachments: &[Some(RenderPassColorAttachment {
        //         view: post_process,
        //         resolve_target: None,
        //         ops: Operations::default(),
        //     })],
        //     depth_stencil_attachment: None,
        //     timestamp_writes: None,
        //     occlusion_query_set: None,
        // });

        // render_pass.set_render_pipeline(pipeline);
        // render_pass.set_bind_group(0, &bind_group, &[]);
        // render_pass.draw(0..3, 0..1);

        Ok(())
    }
}

#[derive(Resource)]
struct PathTracerPipeline {
    layout: BindGroupLayout,
    // sampler: Sampler,
    pipeline_id: CachedComputePipelineId,
}

impl FromWorld for PathTracerPipeline {
    fn from_world(world: &mut World) -> Self {
        let render_device = world.resource::<RenderDevice>();

        let layout: BindGroupLayout = render_device.create_bind_group_layout(
            Some("path_tracer_layout"),
            &BindGroupLayoutEntries::sequential(
                ShaderStages::COMPUTE,
                (
                    texture_storage_2d(TextureFormat::Rgba16Float, StorageTextureAccess::ReadWrite),
                ),
            ),
        );

        // let layout: BindGroupLayout = render_device.create_bind_group_layout(
        //     Some("post_process_bind_group_layout"),
        //     &[
        //         BindGroupLayoutEntry {
        //             binding: 0,
        //             visibility: ShaderStages::FRAGMENT,
        //             ty: BindingType::Texture {
        //                 sample_type: TextureSampleType::Float { filterable: true },
        //                 view_dimension: TextureViewDimension::D2,
        //                 multisampled: false,
        //             },
        //             count: None,
        //         },
        //         BindGroupLayoutEntry {
        //             binding: 1,
        //             visibility: ShaderStages::FRAGMENT,
        //             ty: BindingType::Sampler(SamplerBindingType::Filtering),
        //             count: None,
        //         },
        //         BindGroupLayoutEntry {
        //             binding: 2,
        //             visibility: ShaderStages::FRAGMENT,
        //             ty: BindingType::Buffer {
        //                 ty: bevy::render::render_resource::BufferBindingType::Uniform,
        //                 has_dynamic_offset: false,
        //                 min_binding_size: Some(PathTracerSettings::min_size()),
        //             },
        //             count: None,
        //         },
        //     ],
        // );

        // let sampler = render_device.create_sampler(&SamplerDescriptor::default());

        // The HLSL Registry Holds HLSL shader handles so the file watcher will watch for updates and cause a new spv file to be generated when changes are made.
        // We need at least shader model 6.5 for mesh shaders
        let shader = HLSLRegistry::load_from_world("shaders/path_tracer_entry.hlsl", world, "cs_6_5");

        let pipeline_id =
            world
                .resource_mut::<PipelineCache>()
                .queue_compute_pipeline(ComputePipelineDescriptor  {
                    label: Some("path_tracer".into()),
                    layout: vec![layout.clone()],
                    push_constant_ranges: Vec::new(),
                    shader: shader,
                    shader_defs: vec![],
                    entry_point: Cow::from("main"),
                });

                // Regular shader pipeline (to transform with mesh shaders)
                // let pipeline_id =
                // world
                //     .resource_mut::<PipelineCache>()
                //     .queue_(RenderPipelineDescriptor {
                //         label: Some("post_process_pipeline".into()),
                //         layout: vec![layout.clone()],
    
                //         vertex: fullscreen_shader_vertex_state(),
                //         fragment: Some(FragmentState {
                //             shader,
                //             shader_defs: vec![],
                //             // When selecting a profile with "ps_", fragment will be used for the SPIR-V entry point
                //             entry_point: "fragment".into(),
                //             targets: vec![Some(ColorTargetState {
                //                 format: TextureFormat::bevy_default(),
                //                 blend: None,
                //                 write_mask: ColorWrites::ALL,
                //             })],
                //         }),
    
                //         primitive: PrimitiveState::default(),
                //         depth_stencil: None,
                //         multisample: MultisampleState::default(),
                //         push_constant_ranges: vec![],
                //     });
    
        Self {
            layout,
            // sampler,
            pipeline_id,
        }
    }
}

#[derive(Component, Default, Clone, Copy, ExtractComponent, ShaderType)]
struct PathTracerSettings {
    intensity: f32,
    padding: Vec3,
}

#[derive(Resource, Clone, ExtractResource)]
struct BuiltinResources
{
    color_buffer: Handle<Image>,
}

fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    mut images: ResMut<Assets<Image>>
) {
    commands.spawn((
        Camera3dBundle {
            camera: Camera {
                clear_color: ClearColorConfig::Custom(Color::WHITE),
                ..default()
            },
            transform: Transform::from_translation(Vec3::new(0.0, 0.0, 5.0))
                .looking_at(Vec3::default(), Vec3::Y),
            ..default()
        },
        PathTracerSettings {
            intensity: 0.02,
            ..default()
        },
    ));

    commands.spawn((
        PbrBundle {
            mesh: meshes.add(Cuboid::default()),
            material: materials.add(Color::srgb(0.8, 0.7, 0.6)),
            transform: Transform::from_xyz(0.0, 0.5, 0.0),
            ..default()
        },
        Rotates,
    ));

    commands.spawn(PointLightBundle {
        transform: Transform::from_translation(Vec3::new(0.0, 0.0, 10.0)),
        ..default()
    });

    let mut image = Image::default();
    image.texture_descriptor.format = TextureFormat::Rgba16Float;
    image.texture_descriptor.dimension = TextureDimension::D2;
    image.asset_usage = RenderAssetUsages::RENDER_WORLD;
    image.resize(Extent3d {
        width: WINDOW_RESOLUTION.0,
        height: WINDOW_RESOLUTION.1,
        depth_or_array_layers: 1,
    });

    image.texture_descriptor.usage =
        TextureUsages::COPY_DST | TextureUsages::STORAGE_BINDING | TextureUsages::TEXTURE_BINDING;
    let color_buffer = images.add(image.clone());

    println!("BuiltinResources injected.");
    commands.insert_resource(BuiltinResources {
        color_buffer : color_buffer
    });
}

// TODO: cleanup

#[derive(Component)]
struct Rotates;

fn rotate(time: Res<Time>, mut query: Query<&mut Transform, With<Rotates>>) {
    for mut transform in &mut query {
        transform.rotate_x(0.55 * time.delta_seconds());
        transform.rotate_z(0.15 * time.delta_seconds());
    }
}

fn update_settings(mut settings: Query<&mut PathTracerSettings>, time: Res<Time>) {
    for mut setting in &mut settings {
        let mut intensity = time.elapsed_seconds().sin();

        intensity = intensity.sin();

        intensity = intensity * 0.5 + 0.5;

        intensity *= 0.015;

        setting.intensity = intensity;
    }
}