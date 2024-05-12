use std::{
    path::{Path, PathBuf},
    process::Command,
};

use bevy::{
    asset::{io::Reader, AssetLoader, AssetPath, LoadContext},
    ecs::system::SystemState,
    prelude::*,
    render::RenderApp,
    utils::{BoxedFuture, HashMap},
};
use serde::{Deserialize, Serialize};

pub struct HLSLPlugin;

impl Plugin for HLSLPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<HLSLRegistry>()
            .init_asset::<HLSLShader>()
            .register_asset_loader(HLSLLoader);
        let Some(render_app) = app.get_sub_app_mut(RenderApp) else {
            return;
        };
        render_app.init_resource::<HLSLRegistry>();
    }
}

/// Holds HLSL shader handles so the file watcher will watch for updates and cause a new spv file to be generated when changes are made.
#[derive(Resource, Default)]
pub struct HLSLRegistry(HashMap<PathBuf, Handle<HLSLShader>>);

impl HLSLRegistry {
    /// <profile>: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, ps_6_5, ps_6_6, ps_6_7,
    /// vs_6_0, vs_6_1, vs_6_2, vs_6_3, vs_6_4, vs_6_5, vs_6_6, vs_6_7,
    /// gs_6_0, gs_6_1, gs_6_2, gs_6_3, gs_6_4, gs_6_5, gs_6_6, gs_6_7,
    /// hs_6_0, hs_6_1, hs_6_2, hs_6_3, hs_6_4, hs_6_5, hs_6_6, hs_6_7,
    /// ds_6_0, ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, ds_6_6, ds_6_7,
    /// cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4, cs_6_5, cs_6_6, cs_6_7,
    /// lib_6_1, lib_6_2, lib_6_3, lib_6_4, lib_6_5, lib_6_6, lib_6_7,
    /// ms_6_5, ms_6_6, ms_6_7,
    /// as_6_5, as_6_6, as_6_7,
    pub fn load<'a>(
        &mut self,
        path: impl Into<AssetPath<'a>> + std::marker::Copy,
        asset_server: &AssetServer,
        profile: &str,
    ) -> Handle<Shader> {
        let p: PathBuf = path.into().into();
        // TODO skip this if not using "file_watcher" or "asset_processor" features.
        #[cfg(not(target_arch = "wasm32"))]
        {
            let profile = String::from(profile);
            let h = asset_server.load_with_settings(path, move |s: &mut HLSLSettings| {
                s.profile = profile.clone();
            });
            self.0.insert(p.clone(), h);
        }
        asset_server.load(p.with_extension("spv"))
    }

    /// <profile>: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, ps_6_5, ps_6_6, ps_6_7,
    /// vs_6_0, vs_6_1, vs_6_2, vs_6_3, vs_6_4, vs_6_5, vs_6_6, vs_6_7,
    /// gs_6_0, gs_6_1, gs_6_2, gs_6_3, gs_6_4, gs_6_5, gs_6_6, gs_6_7,
    /// hs_6_0, hs_6_1, hs_6_2, hs_6_3, hs_6_4, hs_6_5, hs_6_6, hs_6_7,
    /// ds_6_0, ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, ds_6_6, ds_6_7,
    /// cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4, cs_6_5, cs_6_6, cs_6_7,
    /// lib_6_1, lib_6_2, lib_6_3, lib_6_4, lib_6_5, lib_6_6, lib_6_7,
    /// ms_6_5, ms_6_6, ms_6_7,
    /// as_6_5, as_6_6, as_6_7,
    pub fn load_from_world<'a>(
        path: impl Into<AssetPath<'a>> + std::marker::Copy,
        world: &mut World,
        profile: &str,
    ) -> Handle<Shader> {
        let mut system_state: SystemState<(Res<AssetServer>, ResMut<HLSLRegistry>)> =
            SystemState::new(world);
        let (asset_server, mut hlsl) = system_state.get_mut(world);
        hlsl.load(path, &asset_server, profile)
    }
}

#[derive(Asset, TypePath, Debug)]
pub struct HLSLShader(pub PathBuf);

#[derive(Default)]
struct HLSLLoader;

#[derive(Default, Serialize, Deserialize)]
struct HLSLSettings {
    profile: String,
}

impl AssetLoader for HLSLLoader {
    type Asset = HLSLShader;
    type Settings = HLSLSettings;
    type Error = std::io::Error;
    fn load<'a>(
        &'a self,
        _reader: &'a mut Reader,
        settings: &'a HLSLSettings,
        load_context: &'a mut LoadContext,
    ) -> BoxedFuture<'a, Result<HLSLShader, Self::Error>> {
        let path = Path::new("assets").join(load_context.asset_path().path());
        let mut cmd = Command::new("dxc");
        // TODO allow custom user config
        cmd.arg(path.clone())
            .arg("-T")
            .arg(&settings.profile)
            .arg("-spirv")
            .arg("-fvk-use-gl-layout")
            .arg("-Fo")
            .arg(path.with_extension("spv"));
        if settings.profile.contains("ps_") {
            cmd.arg("-fspv-entrypoint-name=fragment");
        } else if settings.profile.contains("vs_") {
            cmd.arg("-fspv-entrypoint-name=vertex");
        }
        // Throw error in case the shader compilation fails
        let out: std::process::Output = cmd.output().unwrap();
        if out.stderr.len() > 1 {
            println!("dxc stderr: {}", String::from_utf8_lossy(&out.stderr));
        }

        Box::pin(async move { Ok(HLSLShader(path.to_path_buf())) })
    }

    fn extensions(&self) -> &[&str] {
        &["hlsl"]
    }
}
