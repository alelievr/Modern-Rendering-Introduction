#include "RenderPipeline.hpp"
#include "RenderSettings.hpp"

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
    visibilityTexture->SetName("Visibility Texture");
    ViewDesc outputTextureViewDesc = {};
    outputTextureViewDesc.view_type = ViewType::kRenderTarget;
    outputTextureViewDesc.dimension = ViewDimension::kTexture2D;
    visibilityRenderTargetView = device->CreateView(visibilityTexture, outputTextureViewDesc);
    outputTextureViewDesc.view_type = ViewType::kTexture;
    visibilityTextureView = device->CreateView(visibilityTexture, outputTextureViewDesc);

    // Compute stage allows to bind to every shader stages
    BindKey drawRootConstant = { ShaderType::kCompute, ViewType::kConstantBuffer, 1, 0, 3, UINT32_MAX, true };
    objectLayoutSet = RenderUtils::CreateLayoutSet(device, camera, { drawRootConstant }, RenderUtils::All, RenderUtils::Mesh | RenderUtils::Fragment);
    objectBindingSet = RenderUtils::CreateBindingSet(device, objectLayoutSet, camera, { { drawRootConstant, nullptr } }, RenderUtils::All, RenderUtils::Mesh | RenderUtils::Fragment);
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

void RenderPipeline::FrustumCulling(std::shared_ptr<CommandList> cmd)
{
    cmd->BeginEvent("Frustum Culling");
    auto dxCmd = ((DXCommandList*)cmd.get())->GetCommandList();

    if (!meshletCullingIndirectCountBuffer)
    {
        meshletCullingIndirectCountBuffer = device->CreateBuffer(BindFlag::kUnorderedAccess | BindFlag::kCopyDest, sizeof(uint32_t));
		meshletCullingIndirectCountBuffer->CommitMemory(MemoryType::kDefault);
		meshletCullingIndirectCountBuffer->SetName("Meshlet Culling Indirect Count");
    }

    if (!meshletCullingIndirectCountView)
    {
        ViewDesc viewDesc = {};
        viewDesc.view_type = ViewType::kRWBuffer;
        viewDesc.buffer_format = gli::FORMAT_R32_UINT_PACK32;
        viewDesc.dimension = ViewDimension::kBuffer;
        viewDesc.buffer_size = sizeof(uint32_t);
        viewDesc.structure_stride = sizeof(uint32_t);
        meshletCullingIndirectCountView = device->CreateView(meshletCullingIndirectCountBuffer, viewDesc);
    }

    if (!visibleMeshletsCountBuffer)
    {
        visibleMeshletsCountBuffer = device->CreateBuffer(BindFlag::kUnorderedAccess | BindFlag::kCopyDest, sizeof(uint32_t) * 16);
        visibleMeshletsCountBuffer->CommitMemory(MemoryType::kDefault);
        visibleMeshletsCountBuffer->SetName("Visible Meshlet Count");
    }

    if (!visibleMeshletsCountView)
    {
        ViewDesc viewDesc = {};
        viewDesc.view_type = ViewType::kRWBuffer;
        viewDesc.buffer_format = gli::FORMAT_R32_UINT_PACK32;
        viewDesc.dimension = ViewDimension::kBuffer;
        viewDesc.buffer_size = sizeof(uint32_t) * 16;
        viewDesc.structure_stride = sizeof(uint32_t);
        visibleMeshletsCountView = device->CreateView(visibleMeshletsCountBuffer, viewDesc);
    }

    if (!meshletCullingIndirectArgsBuffer)
    {
        meshletCullingIndirectArgsBuffer = device->CreateBuffer(BindFlag::kIndirectBuffer | BindFlag::kUnorderedAccess, sizeof(IndirectDispatchCommand) * MAX_VISIBLE_MESHLETS);
        meshletCullingIndirectArgsBuffer->CommitMemory(MemoryType::kDefault);
        meshletCullingIndirectArgsBuffer->SetName("Meshlet Culling Indirect Args");
    }

    if (!meshletCullingIndirectArgsView)
    {
        ViewDesc viewDesc = {};
        viewDesc.view_type = ViewType::kRWStructuredBuffer;
        viewDesc.dimension = ViewDimension::kBuffer;
        viewDesc.buffer_size = sizeof(IndirectDispatchCommand) * MAX_VISIBLE_MESHLETS;
        viewDesc.structure_stride = sizeof(IndirectDispatchCommand);
        meshletCullingIndirectArgsView = device->CreateView(meshletCullingIndirectArgsBuffer, viewDesc);
    }

    if (!instanceFrustumCullingLayoutSet)
    {
        BindKey indirectCountKey = { ShaderType::kCompute, ViewType::kRWBuffer, 1, 0 };
        BindKey indirectBindKey = { ShaderType::kCompute, ViewType::kRWStructuredBuffer, 2, 0 };
        BindKey meshletCountKey = { ShaderType::kCompute, ViewType::kRWBuffer, 3, 0 };
        BindKey drawRootConstant = { ShaderType::kCompute, ViewType::kConstantBuffer, 1, 0, 3, UINT32_MAX, true };

        instanceFrustumCullingLayoutSet = RenderUtils::CreateLayoutSet(device, *camera,
            { drawRootConstant, indirectBindKey, indirectCountKey, meshletCountKey },
            RenderUtils::CameraData | RenderUtils::SceneInstances | RenderUtils::MeshPool,
            RenderUtils::Compute
        );
        instanceFrustumCullingSet = RenderUtils::CreateBindingSet(device, instanceFrustumCullingLayoutSet, *camera,
            { { drawRootConstant, nullptr }, { indirectBindKey, meshletCullingIndirectArgsView }, { indirectCountKey, meshletCullingIndirectCountView }, { meshletCountKey, visibleMeshletsCountView } },
            RenderUtils::CameraData | RenderUtils::SceneInstances | RenderUtils::MeshPool,
            RenderUtils::Compute
        );
    }

    if (!frustumCullingProgram.program)
    {
        frustumCullingProgram = RenderUtils::CreateComputePipeline(device, "shaders/InstanceFrustumCulling.hlsl", "main", instanceFrustumCullingLayoutSet);
        frustumCullingClearProgram = RenderUtils::CreateComputePipeline(device, "shaders/InstanceFrustumCulling.hlsl", "clear", instanceFrustumCullingLayoutSet);
        frustumCullingIndirectArgsProgram = RenderUtils::CreateComputePipeline(device, "shaders/InstanceFrustumCulling.hlsl", "updateIndirectArguments", instanceFrustumCullingLayoutSet);
    }

    // Clear previous frame culling data
    cmd->BeginEvent("Reset Visible Meshlet Counter");
    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kCommon, ResourceState::kUnorderedAccess } });

    cmd->BindPipeline(frustumCullingClearProgram.pipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);
    cmd->Dispatch(1, 1, 1);

    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
    cmd->EndEvent();

    cmd->BeginEvent("Frustum Culling");
    cmd->ResourceBarrier({ { Scene::visibleMeshletsBuffer0, ResourceState::kCommon, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kCommon, ResourceState::kUnorderedAccess } });

    cmd->BindPipeline(frustumCullingProgram.pipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);
    // TODO: multiple dispatch if the instance count is too big
    int dispatchCount = (scene->instances.size() + 63) / 64;
    cmd->Dispatch(dispatchCount, 1, 1);

    cmd->ResourceBarrier({ { Scene::visibleMeshletsBuffer0, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
    cmd->EndEvent();

    cmd->BeginEvent("Update Indirect Args");
    cmd->ResourceBarrier({ { meshletCullingIndirectArgsBuffer, ResourceState::kIndirectArgument, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { meshletCullingIndirectCountBuffer, ResourceState::kIndirectArgument, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kCommon, ResourceState::kGenericRead } });

    cmd->BindPipeline(frustumCullingIndirectArgsProgram.pipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);
    cmd->Dispatch(1, 1, 1);

    cmd->ResourceBarrier({ { meshletCullingIndirectArgsBuffer, ResourceState::kUnorderedAccess, ResourceState::kIndirectArgument } });
    cmd->ResourceBarrier({ { meshletCullingIndirectCountBuffer, ResourceState::kUnorderedAccess, ResourceState::kIndirectArgument } });
    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kGenericRead, ResourceState::kCommon } });
    cmd->EndEvent();
    cmd->EndEvent();
}

void RenderPipeline::MeshletCulling(std::shared_ptr<CommandList> cmd)
{
    cmd->BeginEvent("Meshlet Culling");
    auto dxCmd = ((DXCommandList*)cmd.get())->GetCommandList();

    //if (!meshletIndirectCountBuffer)
    //{
    //    meshletIndirectCountBuffer = device->CreateBuffer(BindFlag::kUnorderedAccess | BindFlag::kCopyDest, sizeof(uint32_t));
    //    meshletIndirectCountBuffer->CommitMemory(MemoryType::kDefault);
    //    meshletIndirectCountBuffer->SetName("Meshlet Indirect Count Buffer");
    //}

    //if (!meshletIndirectCountView)
    //{
    //    ViewDesc viewDesc = {};
    //    viewDesc.view_type = ViewType::kRWBuffer;
    //    viewDesc.buffer_format = gli::FORMAT_R32_UINT_PACK32;
    //    viewDesc.dimension = ViewDimension::kBuffer;
    //    viewDesc.buffer_size = sizeof(uint32_t);
    //    viewDesc.structure_stride = sizeof(uint32_t);
    //    meshletIndirectCountView = device->CreateView(meshletIndirectCountBuffer, viewDesc);
    //}

    //if (!meshletIndirectArgsBuffer)
    //{
    //    meshletIndirectArgsBuffer = device->CreateBuffer(BindFlag::kIndirectBuffer | BindFlag::kUnorderedAccess, sizeof(IndirectDispatchCommand) * maxVisibleMeshlets);
    //    meshletIndirectArgsBuffer->CommitMemory(MemoryType::kDefault);
    //    meshletIndirectArgsBuffer->SetName("Meshlet Indirect Args Buffer");
    //}

    //if (!meshletIndirectArgsView)
    //{
    //    ViewDesc viewDesc = {};
    //    viewDesc.view_type = ViewType::kStructuredBuffer;
    //    viewDesc.dimension = ViewDimension::kBuffer;
    //    viewDesc.buffer_size = sizeof(IndirectDispatchCommand) * maxVisibleMeshlets;
    //    viewDesc.structure_stride = sizeof(IndirectDispatchCommand);
    //    meshletIndirectArgsView = device->CreateView(meshletCullingIndirectArgsBuffer, viewDesc);
    //}

    //if (!meshletCullingLayoutSet)
    //{
    //    BindKey indirectBindKey = { ShaderType::kCompute, ViewType::kRWStructuredBuffer, 2, 0 };
    //    BindKey indirectCountKey = { ShaderType::kCompute, ViewType::kRWBuffer, 1, 0 };

    //    meshletCullingLayoutSet = RenderUtils::CreateLayoutSet(device, *camera, { indirectBindKey, indirectCountKey }, RenderUtils::CameraData | RenderUtils::SceneInstances, RenderUtils::Compute);
    //    meshletCullingSet = RenderUtils::CreateBindingSet(device, meshletCullingLayoutSet, *camera,
    //        { { indirectBindKey, meshletIndirectArgsView }, { indirectCountKey, meshletCullingIndirectCountView } },
    //        RenderUtils::CameraData | RenderUtils::SceneInstances, RenderUtils::Compute
    //    );
    //}

    if (!meshletCullingProgram.program)
    {
        meshletCullingProgram = RenderUtils::CreateComputePipeline(device, "shaders/MeshletCulling.hlsl", "main", instanceFrustumCullingLayoutSet);
        meshletCullingClearProgram = RenderUtils::CreateComputePipeline(device, "shaders/MeshletCulling.hlsl", "clear", instanceFrustumCullingLayoutSet);
        meshletCullingIndirectArgsProgram = RenderUtils::CreateComputePipeline(device, "shaders/MeshletCulling.hlsl", "updateIndirectArguments", instanceFrustumCullingLayoutSet);
    }

    if (!meshletCullingCommandSignature)
        meshletCullingCommandSignature = RenderUtils::CreateIndirectRootConstantCommandSignature(device, instanceFrustumCullingLayoutSet, true);

    // Clear previous frame culling data
    cmd->BeginEvent("Reset Visible Meshlet Counter");
    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kCommon, ResourceState::kUnorderedAccess } });

    cmd->BindPipeline(meshletCullingClearProgram.pipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);
    cmd->Dispatch(1, 1, 1);

    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
    cmd->EndEvent();

    cmd->BeginEvent("Meshlet Culling");
    cmd->ResourceBarrier({ { Scene::visibleMeshletsBuffer1, ResourceState::kCommon, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { Scene::visibleMeshletsBuffer0, ResourceState::kCommon, ResourceState::kGenericRead } });

    DXBindingSetLayout* l = (DXBindingSetLayout*)instanceFrustumCullingLayoutSet.get();
    dxCmd->SetComputeRootSignature(l->GetRootSignature().Get());
    cmd->BindPipeline(meshletCullingProgram.pipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);

    DXResource* dxIndirectArgsBuffer = (DXResource*)meshletCullingIndirectArgsBuffer.get();
    DXResource* dxIndirectCountBuffer = (DXResource*)meshletCullingIndirectCountBuffer.get();

    // Instead of drawing objects one by one here, we can use the culling results to render only visible objects
    dxCmd->ExecuteIndirect(
        meshletCullingCommandSignature.Get(),
        MAX_VISIBLE_MESHLETS,
        dxIndirectArgsBuffer->resource.Get(),
        0,
        dxIndirectCountBuffer->resource.Get(),
        0
    );

    cmd->ResourceBarrier({ { Scene::visibleMeshletsBuffer0, ResourceState::kGenericRead, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { Scene::visibleMeshletsBuffer1, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
    cmd->EndEvent();

    cmd->BeginEvent("Update Indirect Args");
    cmd->ResourceBarrier({ { meshletCullingIndirectArgsBuffer, ResourceState::kIndirectArgument, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { meshletCullingIndirectCountBuffer, ResourceState::kIndirectArgument, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kCommon, ResourceState::kGenericRead } });

    cmd->BindPipeline(meshletCullingIndirectArgsProgram.pipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);
    cmd->Dispatch(1, 1, 1);

    cmd->ResourceBarrier({ { visibleMeshletsCountBuffer, ResourceState::kGenericRead, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { meshletCullingIndirectCountBuffer, ResourceState::kUnorderedAccess, ResourceState::kIndirectArgument } });
    cmd->ResourceBarrier({ { meshletCullingIndirectArgsBuffer, ResourceState::kUnorderedAccess, ResourceState::kIndirectArgument } });
    cmd->EndEvent();
    cmd->EndEvent();
}

void RenderPipeline::RenderVisibility(std::shared_ptr<CommandList> cmd)
{
    auto dxCmd = ((DXCommandList*)cmd.get())->GetCommandList();

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

    if (!indirectVisibilitySet)
    {
        //BindKey indirectBindKey = { ShaderType::kCompute, ViewType::kRWStructuredBuffer, 0, 0 };
        BindKey indirectCountKey = { ShaderType::kCompute, ViewType::kRWBuffer, 1, 0 };
        BindKey instanceIDKey = { ShaderType::kCompute, ViewType::kConstantBuffer, 1, 0, 3, UINT32_MAX, true };

        indirectVisibilityLayoutSet = RenderUtils::CreateLayoutSet(device, *camera, { instanceIDKey, indirectCountKey }, RenderUtils::CameraData | RenderUtils::SceneInstances | RenderUtils::MeshPool, RenderUtils::Mesh | RenderUtils::Amplification | RenderUtils::Fragment);
        indirectVisibilitySet = RenderUtils::CreateBindingSet(device, indirectVisibilityLayoutSet, *camera,
            { { instanceIDKey, nullptr }, { indirectCountKey, meshletCullingIndirectCountView } },
            RenderUtils::CameraData | RenderUtils::SceneInstances | RenderUtils::MeshPool, RenderUtils::Mesh | RenderUtils::Amplification | RenderUtils::Fragment
        );
    }

    if (!visibilityPipeline)
    {
        //ShaderDesc visibilityTaskShaderDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/VisibilityPass.hlsl", "task", ShaderType::kAmplification, "6_5" };
        //visibilityTaskShader = device->CompileShader(visibilityTaskShaderDesc);
        ShaderDesc visibilityMeshShaderDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/VisibilityPass.hlsl", "mesh", ShaderType::kMesh, "6_5" };
        visibilityMeshShader = device->CompileShader(visibilityMeshShaderDesc);
        ShaderDesc visibilityFragmentShaderDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/VisibilityPass.hlsl", "fragment", ShaderType::kPixel, "6_5" };
        visibilityFragmentShader = device->CompileShader(visibilityFragmentShaderDesc);

        visibilityProgram = device->CreateProgram({ visibilityMeshShader, visibilityFragmentShader });

        GraphicsPipelineDesc meshShaderPipelineDesc = {
            visibilityProgram,
            indirectVisibilityLayoutSet,
            {},
            visibilityRenderPass,
        };
        meshShaderPipelineDesc.rasterizer_desc = { FillMode::kSolid, CullMode::kBack, 0 };

        visibilityPipeline = device->CreateGraphicsPipeline(meshShaderPipelineDesc);
    }

    if (!frustumCullingCommandSignature)
        frustumCullingCommandSignature = RenderUtils::CreateIndirectRootConstantCommandSignature(device, indirectVisibilityLayoutSet, false);

    cmd->BeginEvent("Visibility Pass");
    cmd->ResourceBarrier({ { visibilityTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite}});

    cmd->BeginRenderPass(visibilityRenderPass, visibilityFrameBuffer, {});

    cmd->BindPipeline(visibilityPipeline);
    cmd->BindBindingSet(indirectVisibilitySet);

    //DrawOpaqueObjects(cmd, objectBindingSet, visibilityPipeline);

    DXResource* dxIndirectArgsBuffer = (DXResource*)meshletCullingIndirectArgsBuffer.get();
    DXResource* dxIndirectCountBuffer = (DXResource*)meshletCullingIndirectCountBuffer.get();

    // Instead of drawing objects one by one here, we can use the culling results to render only visible objects
    dxCmd->ExecuteIndirect(
        frustumCullingCommandSignature.Get(),
        MAX_VISIBLE_MESHLETS,
        dxIndirectArgsBuffer->resource.Get(),
        0,
        dxIndirectCountBuffer->resource.Get(),
        0
    );

    cmd->EndRenderPass();
    
    cmd->ResourceBarrier({ { visibilityTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kDepthStencilWrite, ResourceState::kCommon } });
    cmd->EndEvent();
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
        BindKey visibilityTexture = { ShaderType::kPixel, ViewType::kTexture, 1, 2 };
        forwardLayoutSet = RenderUtils::CreateLayoutSet(device, *camera, { drawRootConstant, visibilityTexture }, RenderUtils::All, RenderUtils::Mesh | RenderUtils::Fragment);
        forwardBindingSet = RenderUtils::CreateBindingSet(device, forwardLayoutSet, *camera, { { drawRootConstant, nullptr }, { visibilityTexture, visibilityTextureView } }, RenderUtils::All, RenderUtils::Mesh | RenderUtils::Fragment);
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

    cmd->BeginEvent("Forward Opaque Pass");
    cmd->ResourceBarrier({ { colorTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite} });

    cmd->BeginRenderPass(forwardRenderPass, forwardFrameBuffer, {});

    // Draw fullscreen forward pass
    cmd->BindPipeline(forwardPipeline);
    cmd->BindBindingSet(forwardBindingSet);
    cmd->DispatchMesh(1);

    cmd->EndRenderPass();

    cmd->ResourceBarrier({ { colorTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kDepthStencilWrite, ResourceState::kCommon } });
    cmd->EndEvent();
}

void RenderPipeline::Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, std::shared_ptr<Scene> scene)
{
	this->scene = scene;

    // Frustum cull instances of the scene using their OBB
    // Outputs a buffer of visible meshlets
    FrustumCulling(cmd);

    // Cull the meshlets usig frustum and cone culling
    // Outputs a reduced list of visible meshlets for the next render passes
    MeshletCulling(cmd);

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
