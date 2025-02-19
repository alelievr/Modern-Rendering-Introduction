#include "RenderPipeline.hpp"
#include <CommandList/DXCommandList.h>
#include "RenderUtils.hpp"

RenderPipeline::RenderPipeline(std::shared_ptr<Device> device, const AppSize& appSize,
    Camera& camera, std::shared_ptr<Resource> colorTexture, std::shared_ptr<View> colorTextureView,
    std::shared_ptr<Resource> depthTexture, std::shared_ptr<View> depthTextureView)
{
    this->camera = &camera;
    this->device = device;
    this->colorTexture = colorTexture;
    this->colorTextureView = colorTextureView;
    this->depthTexture = depthTexture;
    this->depthTextureView = depthTextureView;
    this->appSize = appSize;

    visibilityTexture = device->CreateTexture(TextureType::k2D, BindFlag::kRenderTarget | BindFlag::kUnorderedAccess | BindFlag::kShaderResource | BindFlag::kCopySource, gli::format::FORMAT_R32_UINT_PACK32, 1, appSize.width(), appSize.height(), 1, 1);
    visibilityTexture->CommitMemory(MemoryType::kDefault);
    visibilityTexture->SetName("ColorTexture");
    ViewDesc outputTextureViewDesc = {};
    outputTextureViewDesc.view_type = ViewType::kRenderTarget;
    outputTextureViewDesc.dimension = ViewDimension::kTexture2D;
    visibilityRenderTargetView = device->CreateView(visibilityTexture, outputTextureViewDesc);
    outputTextureViewDesc.view_type = ViewType::kTexture;
    visibilityTextureView = device->CreateView(visibilityTexture, outputTextureViewDesc);

    // Compute stage allows to bind to every shader stages
    BindKey drawRootConstant = { ShaderType::kCompute, ViewType::kConstantBuffer, 1, 0, 3, UINT32_MAX, true };
    objectLayoutSet = RenderUtils::CreateLayoutSet(device, camera, { drawRootConstant }, RenderUtils::All);
    objectBindingSet = RenderUtils::CreateBindingSet(device, objectLayoutSet, camera, { { drawRootConstant, nullptr } }, RenderUtils::All);
}

void RenderPipeline::DrawOpaqueObjects(std::shared_ptr<CommandList> cmd, std::shared_ptr<BindingSet> set, std::shared_ptr<Pipeline> pipeline)
{
    cmd->BindPipeline(pipeline);
    cmd->BindBindingSet(set);

    // Get the native command list to push constants directly to the commnand buffer.
    // This allows to have a unique index per draw without having to bind a new constant buffer.
    auto dxCmd = (DXCommandList*)cmd.get();

    // TODO: batch per materials to avoid pipeline switches & reduce CBuffer updates
    for (auto& instance : scene->instances)
    {
        dxCmd->SetGraphicsConstant(0, instance.instanceDataOffset, 2);

        for (auto& r : instance.model.parts)
        {
            // TODO: mesh index when meshlets are supported
            int materialIndex = r.material->materialIndex;

            // Bind per-draw data, we only need an index, the rest is bindless
            dxCmd->SetGraphicsConstant(0, materialIndex, 0);
            dxCmd->SetGraphicsConstant(0, r.mesh->meshletOffset, 1);

            cmd->DispatchMesh(r.mesh->meshletCount);
        }
    }
}

void RenderPipeline::RenderVisibility(std::shared_ptr<CommandList> cmd)
{
    if (!visibilityRenderPass)
    {
        // Create Visibility Render passes
        RenderPassDepthStencilDesc depthStencilDesc = {
            gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64,
            RenderPassLoadOp::kClear, RenderPassStoreOp::kStore,
            RenderPassLoadOp::kClear, RenderPassStoreOp::kStore
        };
        RenderPassColorDesc visibilityDesc = { gli::FORMAT_R32_UINT_PACK32, RenderPassLoadOp::kClear, RenderPassStoreOp::kStore };
        RenderPassDesc renderPassDesc = {
            { visibilityDesc },
            depthStencilDesc
        };
        visibilityRenderPass = device->CreateRenderPass(renderPassDesc);
    }

    if (!visibilityFrameBuffer)
    {
        FramebufferDesc desc = {};
        desc.render_pass = visibilityRenderPass;
        desc.width = appSize.width();
        desc.height = appSize.height();
        desc.colors = { visibilityRenderTargetView };
        desc.depth_stencil = depthTextureView;

        visibilityFrameBuffer = device->CreateFramebuffer(desc);
    }

    if (!visibilityPipeline)
    {
        ShaderDesc visibilityMeshShaderDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/VisibilityPass.hlsl", "mesh", ShaderType::kMesh, "6_5" };
        std::shared_ptr<Shader> visibilityMeshShader = device->CompileShader(visibilityMeshShaderDesc);
        ShaderDesc visibilityFragmentShaderDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/VisibilityPass.hlsl", "fragment", ShaderType::kPixel, "6_5" };
        std::shared_ptr<Shader> visibilityFragmentShader = device->CompileShader(visibilityFragmentShaderDesc);

        visibilityProgram = device->CreateProgram({ visibilityMeshShader, visibilityFragmentShader });

        GraphicsPipelineDesc meshShaderPipelineDesc = {
            visibilityProgram,
            objectLayoutSet,
            {},
            visibilityRenderPass,
        };
        meshShaderPipelineDesc.rasterizer_desc = { FillMode::kSolid, CullMode::kBack, 0 };

        visibilityPipeline = device->CreateGraphicsPipeline(meshShaderPipelineDesc);
    }

    ClearDesc clearDesc = { { { 0.0, 0.2, 0.4, 1.0 } } }; // Clear Color
    cmd->ResourceBarrier({ { visibilityTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite}});

    cmd->BeginRenderPass(visibilityRenderPass, visibilityFrameBuffer, clearDesc);

    DrawOpaqueObjects(cmd, objectBindingSet, visibilityPipeline);

    cmd->EndRenderPass();
    
    cmd->ResourceBarrier({ { visibilityTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kDepthStencilWrite, ResourceState::kCommon } });
}

void RenderPipeline::RenderForwardOpaque(std::shared_ptr<CommandList> cmd)
{
    if (!forwardRenderPass)
    {
        RenderPassDepthStencilDesc depthStencilDesc = {
            gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64,
            RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore,
            RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore
        };
        RenderPassColorDesc colorDesc = { gli::FORMAT_RGBA8_UNORM_PACK8, RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore };
        RenderPassDesc renderPassDesc = {
            { colorDesc },
            depthStencilDesc
        };
        forwardRenderPass = device->CreateRenderPass(renderPassDesc);
    }

    if (!forwardFrameBuffer)
    {
        FramebufferDesc desc = {};
        desc.render_pass = forwardRenderPass;
        desc.width = appSize.width();
        desc.height = appSize.height();
        desc.colors = { colorTextureView };
        desc.depth_stencil = depthTextureView;

        forwardFrameBuffer = device->CreateFramebuffer(desc);
    }

    if (!forwardLayoutSet)
    {
        // Compute stage allows to bind to every shader stages
        BindKey drawRootConstant = { ShaderType::kCompute, ViewType::kConstantBuffer, 1, 0, 3, UINT32_MAX, true };
        BindKey visibilityTexture = { ShaderType::kPixel, ViewType::kTexture, 0, 4 };
        forwardLayoutSet = RenderUtils::CreateLayoutSet(device, *camera, { drawRootConstant, visibilityTexture }, RenderUtils::All);
        forwardBindingSet = RenderUtils::CreateBindingSet(device, forwardLayoutSet, *camera, { { drawRootConstant, nullptr }, { visibilityTexture, visibilityTextureView } }, RenderUtils::All);
    }

    if (!forwardPipeline)
    {
        ShaderDesc forwardMeshShaderDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/ForwardPass.hlsl", "mesh", ShaderType::kMesh, "6_5" };
        std::shared_ptr<Shader> forwardMeshShader = device->CompileShader(forwardMeshShaderDesc);
        ShaderDesc forwardFragmentShaderDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/ForwardPass.hlsl", "fragment", ShaderType::kPixel, "6_5" };
        std::shared_ptr<Shader> forwardFragmentShader = device->CompileShader(forwardFragmentShaderDesc);

        forwardProgram = device->CreateProgram({ forwardMeshShader, forwardFragmentShader });

        GraphicsPipelineDesc meshShaderPipelineDesc = {
            forwardProgram,
            forwardLayoutSet,
            {},
            forwardRenderPass,
        };
        meshShaderPipelineDesc.rasterizer_desc = { FillMode::kSolid, CullMode::kBack, 0 };
        meshShaderPipelineDesc.depth_stencil_desc = { true, ComparisonFunc::kAlways, false };

        forwardPipeline = device->CreateGraphicsPipeline(meshShaderPipelineDesc);
    }

    ClearDesc clearDesc = { { { 0.0, 0.2, 0.4, 1.0 } } }; // Clear Color
    cmd->ResourceBarrier({ { colorTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite} });

    cmd->BeginRenderPass(forwardRenderPass, forwardFrameBuffer, clearDesc);

    // Draw fullscreen forward pass
    cmd->BindPipeline(forwardPipeline);
    cmd->BindBindingSet(forwardBindingSet);
    cmd->DispatchMesh(1);

    cmd->EndRenderPass();

    cmd->ResourceBarrier({ { colorTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kDepthStencilWrite, ResourceState::kCommon } });
}

void RenderPipeline::Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, std::shared_ptr<Scene> scene)
{
	this->scene = scene;

    // TODO: frustum culling

    // Visibility pass:
    // Clears depth, draw into depth and visibility targets
    // TODO: two-pass occlusion culling
    RenderVisibility(cmd);

    // TODO: build lighting structures + shadows

    // Forward opaque pass:
    // Transforms visibility information direclty into color
    // TODO: Gbuffer path
    RenderForwardOpaque(cmd);

    // Render sky where no opaque objects are visible
    scene->sky.Render(cmd, colorTexture, colorTextureView, depthTexture, depthTextureView);

    // TODO: transparency
}
