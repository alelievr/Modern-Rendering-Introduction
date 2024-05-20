//! A compute shader that simulates Conway's Game of Life.
//!
//! Compute shaders use the GPU for computing arbitrary information, that may be independent of what
//! is rendered to the screen.

use bevy::{
    prelude::*,
    render::{
        extract_resource::{ExtractResource, ExtractResourcePlugin}, render_asset::{RenderAssetUsages, RenderAssets}, render_graph::{self, RenderGraph, RenderLabel}, render_resource::{binding_types::texture_storage_2d, *}, renderer::{RenderContext, RenderDevice}, texture::GpuImage, view::{ExtractedWindows, ViewTarget}, Render, RenderApp, RenderSet
    },
};
use bevy_mod_hlsl::HLSLShader;
use std::borrow::Cow;

use bevy_mod_hlsl::{HLSLPlugin, HLSLRegistry};

const WINDOW_RESOLUTION: (u32, u32) = (1920, 1080);
const WORKGROUP_SIZE: u32 = 8;

fn main() {
    App::new()
        .insert_resource(ClearColor(Color::BLACK))
        .add_plugins((
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
        .run();
}

fn setup(mut commands: Commands, mut images: ResMut<Assets<Image>>) {
    let mut image = Image::new_fill(
        Extent3d {
            width: WINDOW_RESOLUTION.0,
            height: WINDOW_RESOLUTION.1,
            depth_or_array_layers: 1,
        },
        TextureDimension::D2,
        &[0, 0, 0, 255],
        TextureFormat::R32Float,
        RenderAssetUsages::RENDER_WORLD,
    );
    image.texture_descriptor.usage =
        TextureUsages::COPY_DST | TextureUsages::STORAGE_BINDING | TextureUsages::TEXTURE_BINDING;
    let image0 = images.add(image.clone());
    let image1 = images.add(image);

    commands.spawn(SpriteBundle {
        sprite: Sprite {
            custom_size: Some(Vec2::new(WINDOW_RESOLUTION.0 as f32, WINDOW_RESOLUTION.1 as f32)),
            ..default()
        },
        texture: image0.clone(),
        transform: Transform::from_scale(Vec3::splat(1f32)),
        ..default()
    });
    commands.spawn(Camera2dBundle::default());

    commands.insert_resource(PathTracerResources {
        texture_a: image0,
        texture_b: image1,
    });
}

struct PathTracerPlugin;

#[derive(Debug, Hash, PartialEq, Eq, Clone, RenderLabel)]
struct PathTracerLabel;

impl Plugin for PathTracerPlugin {
    fn build(&self, app: &mut App) {
        // Extract the game of life image resource from the main world into the render world
        // for operation on by the compute shader and display on the sprite.
        app.add_plugins(ExtractResourcePlugin::<PathTracerResources>::default());
        let render_app = app.sub_app_mut(RenderApp);
        render_app.add_systems(
            Render,
            prepare_bind_group.in_set(RenderSet::PrepareBindGroups),
        );

        let mut render_graph = render_app.world_mut().resource_mut::<RenderGraph>();
        render_graph.add_node(PathTracerLabel, PathTracerNode::default());
        render_graph.add_node_edge(PathTracerLabel, bevy::render::graph::CameraDriverLabel);
    }

    fn finish(&self, app: &mut App) {
        let render_app = app.sub_app_mut(RenderApp);
        render_app.init_resource::<PathTracerPipeline>();
    }
}

#[derive(Resource, Clone, ExtractResource)]
struct PathTracerResources {
    texture_a: Handle<Image>,
    texture_b: Handle<Image>,
}

#[derive(Resource)]
struct PathTracerBindGroups([BindGroup; 2]);

fn prepare_bind_group(
    mut commands: Commands,
    pipeline: Res<PathTracerPipeline>,
    gpu_images: Res<RenderAssets<GpuImage>>,
    game_of_life_images: Res<PathTracerResources>,
    render_device: Res<RenderDevice>
) {
    let view_a = gpu_images.get(&game_of_life_images.texture_a).unwrap();
    let view_b = gpu_images.get(&game_of_life_images.texture_b).unwrap();
    let bind_group_0 = render_device.create_bind_group(
        None,
        &pipeline.texture_bind_group_layout,
        &BindGroupEntries::sequential((&view_a.texture_view, &view_b.texture_view)),
    );

    let bind_group_1 = render_device.create_bind_group(
        None,
        &pipeline.texture_bind_group_layout,
        &BindGroupEntries::sequential((&view_b.texture_view, &view_a.texture_view)),
    );
    commands.insert_resource(PathTracerBindGroups([bind_group_0, bind_group_1]));
}

#[derive(Resource)]
struct PathTracerPipeline {
    texture_bind_group_layout: BindGroupLayout,
    init_pipeline: CachedComputePipelineId,
    update_pipeline: CachedComputePipelineId,
}

impl FromWorld for PathTracerPipeline {
    fn from_world(world: &mut World) -> Self {
        let render_device = world.resource::<RenderDevice>();
        let texture_bind_group_layout = render_device.create_bind_group_layout(
            "Path Tracer",
            &BindGroupLayoutEntries::sequential(
                ShaderStages::COMPUTE,
                (
                    texture_storage_2d(TextureFormat::R32Float, StorageTextureAccess::ReadOnly),
                    texture_storage_2d(TextureFormat::R32Float, StorageTextureAccess::WriteOnly),
                ),
            ),
        );
        let shader = world.load_asset("shaders/game_of_life.wgsl");
        let shader2 = HLSLRegistry::load_from_world("shaders/path_tracer_entry.hlsl", world, "cs_6_5");
        let pipeline_cache = world.resource::<PipelineCache>();
        let init_pipeline = pipeline_cache.queue_compute_pipeline(ComputePipelineDescriptor {
            label: None,
            layout: vec![texture_bind_group_layout.clone()],
            push_constant_ranges: Vec::new(),
            shader: shader.clone(),
            shader_defs: vec![],
            entry_point: Cow::from("init"),
        });
        let update_pipeline = pipeline_cache.queue_compute_pipeline(ComputePipelineDescriptor {
            label: None,
            layout: vec![texture_bind_group_layout.clone()],
            push_constant_ranges: Vec::new(),
            shader,
            shader_defs: vec![],
            entry_point: Cow::from("update"),
        });

        PathTracerPipeline {
            texture_bind_group_layout,
            init_pipeline,
            update_pipeline,
        }
    }
}

struct PathTracerNode;

impl Default for PathTracerNode {
    fn default() -> Self {
        Self {
        }
    }
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
        let bind_groups = &world.resource::<PathTracerBindGroups>().0;
        let pipeline_cache = world.resource::<PipelineCache>();
        let pipeline = world.resource::<PathTracerPipeline>();

        let mut pass = render_context
            .command_encoder()
            .begin_compute_pass(&ComputePassDescriptor::default());

        pass.push_debug_group("Reference Path Tracer");

        let update_pipeline = pipeline_cache
            .get_compute_pipeline(pipeline.update_pipeline)
            .unwrap();
        pass.set_bind_group(0, &bind_groups[1], &[]);
        pass.set_pipeline(update_pipeline);
        pass.dispatch_workgroups(WINDOW_RESOLUTION.0 / WORKGROUP_SIZE, WINDOW_RESOLUTION.1 / WORKGROUP_SIZE, 1);

        pass.pop_debug_group();

        Ok(())
    }
}