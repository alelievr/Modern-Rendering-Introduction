#include "RenderPipeline.hpp"
#include "RenderUtils.hpp"
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

struct IndirectDispatchCommand
{
    unsigned instanceID; // Custom data for the amplification shader to find the instance back
    D3D12_DISPATCH_ARGUMENTS dispatchArgs;
};

void RenderPipeline::FrustumCulling(std::shared_ptr<CommandList> cmd)
{
    auto dxCmd = ((DXCommandList*)cmd.get())->GetCommandList();

    if (!meshletIndirectCountBuffer)
    {
        meshletIndirectCountBuffer = device->CreateBuffer(BindFlag::kUnorderedAccess | BindFlag::kCopyDest, sizeof(uint32_t));
		meshletIndirectCountBuffer->CommitMemory(MemoryType::kDefault);
		meshletIndirectCountBuffer->SetName("Meshlet Indirect Count Buffer");
    }

    if (!meshletIndirectCountBufferView)
    {
        ViewDesc viewDesc = {};
        viewDesc.view_type = ViewType::kRWBuffer;
        viewDesc.buffer_format = gli::FORMAT_R32_UINT_PACK32;
        viewDesc.dimension = ViewDimension::kBuffer;
        viewDesc.buffer_size = sizeof(uint32_t);
        viewDesc.structure_stride = sizeof(uint32_t);
        meshletIndirectCountBufferView = device->CreateView(meshletIndirectCountBuffer, viewDesc);
    }

    if (!meshletIndirectArgsBuffer)
    {
        meshletIndirectArgsBuffer = device->CreateBuffer(BindFlag::kIndirectBuffer | BindFlag::kUnorderedAccess, sizeof(IndirectDispatchCommand) * scene->instances.size());
        meshletIndirectArgsBuffer->CommitMemory(MemoryType::kDefault);
        meshletIndirectArgsBuffer->SetName("Meshlet Indirect Args Buffer");
    }

    if (!meshletIndirectArgsBufferView)
    {
        ViewDesc viewDesc = {};
        viewDesc.view_type = ViewType::kStructuredBuffer;
        viewDesc.dimension = ViewDimension::kBuffer;
        viewDesc.buffer_size = sizeof(IndirectDispatchCommand) * scene->instances.size();
        viewDesc.structure_stride = sizeof(IndirectDispatchCommand);
        meshletIndirectArgsBufferView = device->CreateView(meshletIndirectArgsBuffer, viewDesc);
    }

    if (!instanceFrustumCullingLayoutSet)
    {
        BindKey indirectBindKey = { ShaderType::kCompute, ViewType::kRWStructuredBuffer, 2, 0 };
        BindKey indirectCountKey = { ShaderType::kCompute, ViewType::kRWBuffer, 1, 0 };

        instanceFrustumCullingLayoutSet = RenderUtils::CreateLayoutSet(device, *camera, { indirectBindKey, indirectCountKey }, RenderUtils::CameraData | RenderUtils::SceneInstances, RenderUtils::Compute);
        instanceFrustumCullingSet = RenderUtils::CreateBindingSet(device, instanceFrustumCullingLayoutSet, *camera,
            { { indirectBindKey, meshletIndirectArgsBufferView }, { indirectCountKey, meshletIndirectCountBufferView } },
            RenderUtils::CameraData | RenderUtils::SceneInstances, RenderUtils::Compute
        );
    }

    if (!frustumCullingProgram)
    {
        ShaderDesc instanceFrustumCullingDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/InstanceFrustumCulling.hlsl", "main", ShaderType::kCompute, "6_5" };
        ShaderDesc instanceFrustumCullingClearDesc = { MODERN_RENDERER_ASSETS_PATH "shaders/InstanceFrustumCulling.hlsl", "clear", ShaderType::kCompute, "6_5" };
        frustumCullingShader = device->CompileShader(instanceFrustumCullingDesc);
        frustumCullingClearShader = device->CompileShader(instanceFrustumCullingClearDesc);
        frustumCullingProgram = device->CreateProgram({ frustumCullingShader });
        frustumCullingClearProgram = device->CreateProgram({ frustumCullingClearShader });

        ComputePipelineDesc frustumCullingPipelineDesc = {
            frustumCullingProgram,
            instanceFrustumCullingLayoutSet,
        };

        ComputePipelineDesc frustumCullingClearPipelineDesc = {
            frustumCullingClearProgram,
            instanceFrustumCullingLayoutSet,
        };

        frustumCullingPipeline = device->CreateComputePipeline(frustumCullingPipelineDesc);
        frustumCullingClearPipeline = device->CreateComputePipeline(frustumCullingClearPipelineDesc);
    }

    // Clear previous frame culling data
    cmd->ResourceBarrier({ { meshletIndirectCountBuffer, ResourceState::kCommon, ResourceState::kUnorderedAccess } });

    cmd->BindPipeline(frustumCullingClearPipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);
    cmd->Dispatch(1, 1, 1);

    cmd->ResourceBarrier({ { meshletIndirectCountBuffer, ResourceState::kUnorderedAccess, ResourceState::kCommon } });

    cmd->ResourceBarrier({ { meshletIndirectArgsBuffer, ResourceState::kIndirectArgument, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { meshletIndirectCountBuffer, ResourceState::kCommon, ResourceState::kUnorderedAccess } });

    cmd->BindPipeline(frustumCullingPipeline);
    cmd->BindBindingSet(instanceFrustumCullingSet);
    // TODO: 2D or 3D dispatch if the instance count is too big
    int dispatchCount = (scene->instances.size() + 63) / 64;
    cmd->Dispatch(dispatchCount, 1, 1);

    cmd->ResourceBarrier({ { meshletIndirectCountBuffer, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { meshletIndirectArgsBuffer, ResourceState::kUnorderedAccess, ResourceState::kIndirectArgument } });
}

void RenderPipeline::RenderVisibility(std::shared_ptr<CommandList> cmd)
{
    auto dxCmd = ((DXCommandList*)cmd.get())->GetCommandList();
    auto dxDevice = (DXDevice*)device.get();
    auto nativeDevice = dxDevice->GetDevice();

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

        indirectVisibilityLayoutSet = RenderUtils::CreateLayoutSet(device, *camera, { instanceIDKey, indirectCountKey }, RenderUtils::CameraData | RenderUtils::SceneInstances | RenderUtils::MeshPool, RenderUtils::Mesh | RenderUtils::Fragment);
        indirectVisibilitySet = RenderUtils::CreateBindingSet(device, indirectVisibilityLayoutSet, *camera,
            { { instanceIDKey, nullptr }, { indirectCountKey, meshletIndirectCountBufferView } },
            RenderUtils::CameraData | RenderUtils::SceneInstances | RenderUtils::MeshPool, RenderUtils::Mesh | RenderUtils::Fragment
        );
    }

    if (!visibilityPipeline)
    {
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
    {
        // Define the indirect argument descriptors
        D3D12_INDIRECT_ARGUMENT_DESC args[2] = {};

        // Root constant to pass the instanceID
        args[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
        args[0].Constant.RootParameterIndex = 0;
        args[0].Constant.Num32BitValuesToSet = 1;
        args[0].Constant.DestOffsetIn32BitValues = 2;

        // Dispatch mesh arguments
        args[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

        // Create the command signature description
        D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
        commandSignatureDesc.ByteStride = sizeof(IndirectDispatchCommand);
        commandSignatureDesc.NumArgumentDescs = _countof(args);
        commandSignatureDesc.pArgumentDescs = args;

        DXBindingSetLayout* layout = (DXBindingSetLayout*)indirectVisibilityLayoutSet.get();

        // Create the command signature
        HRESULT hr = nativeDevice->CreateCommandSignature(
            &commandSignatureDesc,
            layout->GetRootSignature().Get(),
            IID_PPV_ARGS(&frustumCullingCommandSignature)
        );

        if (FAILED(hr))
            printf("Failed to create command signature\n");
    }

    ClearDesc clearDesc = { { { 0.0, 0.2, 0.4, 1.0 } } }; // Clear Color
    cmd->ResourceBarrier({ { visibilityTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite}});

    cmd->BeginRenderPass(visibilityRenderPass, visibilityFrameBuffer, clearDesc);

    cmd->BindPipeline(visibilityPipeline);
    cmd->BindBindingSet(indirectVisibilitySet);

    //DrawOpaqueObjects(cmd, objectBindingSet, visibilityPipeline);

    DXResource* dxIndirectArgsBuffer = (DXResource*)meshletIndirectArgsBuffer.get();
    DXResource* dxIndirectCountBuffer = (DXResource*)meshletIndirectCountBuffer.get();

    // Instead of drawing objects one by one here, we can use the culling results to render only visible objects
    dxCmd->ExecuteIndirect(
        frustumCullingCommandSignature.Get(),
        scene->instances.size(),
        dxIndirectArgsBuffer->resource.Get(),
        0,
        dxIndirectCountBuffer->resource.Get(),
        0
    );

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

    // Frustum cull instances of the scene using their OBB
    // The result goes directly into an indirect argument buffer for the amplification shaders
    if (!RenderSettings::freezeFrustumCulling)
        FrustumCulling(cmd);

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
