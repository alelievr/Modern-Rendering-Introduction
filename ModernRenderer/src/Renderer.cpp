#include "Renderer.hpp"
#include "RenderDoc.hpp"
#include <CommandList/DXCommandList.h>
#include "RenderUtils.hpp"
#include "RenderSettings.hpp"
#include "Profiler.hpp"

Renderer::Renderer(std::shared_ptr<Device> device, AppBox& app, Camera& camera)
{
    this->device = device;
    this->appSize = app.GetAppSize();
    this->camera = &camera;
    imGUI = std::make_shared<ImGUIRenderPass>(device, app);

    Profiler::Init(device);

	app.SubscribeEvents((InputEvents*)&controls, nullptr);

    AllocateRenderTargets();
    CompileShaders();
    CreatePipelineObjects();

    renderPipeline = std::make_shared<RenderPipeline>(device, app.GetAppSize(), camera, mainColorTexture, mainColorRenderTargetView, mainDepthTexture, mainDepthTextureView);
}

Renderer::~Renderer()
{
}

void Renderer::AllocateRenderTargets()
{
    // Allocate the RTs for the render pipeline
    mainColorTexture = device->CreateTexture(TextureType::k2D, BindFlag::kRenderTarget | BindFlag::kUnorderedAccess | BindFlag::kShaderResource | BindFlag::kCopySource, gli::format::FORMAT_RGBA8_UNORM_PACK8, 1, appSize.width(), appSize.height(), 1, 1);
    mainColorTexture->CommitMemory(MemoryType::kDefault);
    mainColorTexture->SetName("ColorTexture");
    ViewDesc outputTextureViewDesc = {};
    outputTextureViewDesc.view_type = ViewType::kRWTexture;
    outputTextureViewDesc.dimension = ViewDimension::kTexture2D;
    mainColorTextureView = device->CreateView(mainColorTexture, outputTextureViewDesc);
    outputTextureViewDesc.view_type = ViewType::kRenderTarget;
    mainColorRenderTargetView = device->CreateView(mainColorTexture, outputTextureViewDesc);

    mainDepthTexture = device->CreateTexture(TextureType::k2D, BindFlag::kDepthStencil | BindFlag::kShaderResource, gli::format::FORMAT_D32_SFLOAT_S8_UINT_PACK64, 1, appSize.width(), appSize.height(), 1, 1);
    mainDepthTexture->CommitMemory(MemoryType::kDefault);
    mainDepthTexture->SetName("DepthTexture");
    ViewDesc depth2DDesc = {};
    depth2DDesc.view_type = ViewType::kDepthStencil;
    depth2DDesc.dimension = ViewDimension::kTexture2D;
    mainDepthTextureView = device->CreateView(mainDepthTexture, depth2DDesc);

    pathTracingAccumulationTexture = device->CreateTexture(TextureType::k2D, BindFlag::kRenderTarget | BindFlag::kUnorderedAccess | BindFlag::kShaderResource | BindFlag::kCopySource, gli::format::FORMAT_RGBA32_SFLOAT_PACK32, 1, appSize.width(), appSize.height(), 1, 1);
    pathTracingAccumulationTexture->CommitMemory(MemoryType::kDefault);
    pathTracingAccumulationTexture->SetName("Path Tracing Accumulation");

    ViewDesc pathTracingAccumulationViewDesc = {};
    pathTracingAccumulationViewDesc.view_type = ViewType::kRWTexture;
    pathTracingAccumulationViewDesc.dimension = ViewDimension::kTexture2D;
    pathTracingAccumulationView = device->CreateView(pathTracingAccumulationTexture, pathTracingAccumulationViewDesc);

    // Create the framebuffer
    ViewDesc stencil2DDesc = {};
    stencil2DDesc.view_type = ViewType::kDepthStencil;
    stencil2DDesc.dimension = ViewDimension::kTexture2D;

    FramebufferDesc desc = {};
    desc.render_pass = loadStoreColorRenderPass;
    desc.width = appSize.width();
    desc.height = appSize.height();
    desc.colors = { mainColorRenderTargetView };
    desc.depth_stencil = nullptr;
    imGUIFrameBuffer = device->CreateFramebuffer(desc);

    if (!RenderSettings::noUI)
        imGUIPass = device->CreateRenderPass({ { { mainColorRenderTargetView->GetResource()->GetFormat(), RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore}}});
}

void Renderer::CompileShaders()
{
    // Create HW path tracing program
    pathTracingLibrary = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/RayTracing.hlsl", "", ShaderType::kLibrary, "6_5" });
    pathTracingHitLibrary = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/RayTracingHit.hlsl", "", ShaderType::kLibrary, "6_5" });
    pathTracingMissLibrary = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/RayTracingMiss.hlsl", "", ShaderType::kLibrary, "6_5" });
    pathTracingProgram = device->CreateProgram({ pathTracingLibrary, pathTracingHitLibrary, pathTracingMissLibrary });
}

void Renderer::CreatePipelineObjects()
{
    // Create Render passes
    RenderPassDepthStencilDesc depthStencilDesc = {
        gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64,
        RenderPassLoadOp::kClear, RenderPassStoreOp::kStore, // depth load/store
        RenderPassLoadOp::kClear, RenderPassStoreOp::kStore // stencil load/store
    };
    RenderPassColorDesc mainColorDesc = { gli::FORMAT_RGBA8_UNORM_PACK8, RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore };
    RenderPassDesc renderPassDesc = {
        { mainColorDesc },
        depthStencilDesc
    };
    loadStoreColorRenderPass = device->CreateRenderPass(renderPassDesc);

    // Create the clear render pass setup
    for (auto& d : renderPassDesc.colors)
        d.load_op = RenderPassLoadOp::kClear;
    renderPassDesc.depth_stencil.depth_load_op = RenderPassLoadOp::kClear;
    renderPassDesc.depth_stencil.stencil_load_op = RenderPassLoadOp::kClear;
    clearColorRenderPass = device->CreateRenderPass(renderPassDesc);

    BindKey drawRootConstant = { ShaderType::kCompute, ViewType::kConstantBuffer, 1, 0, 3, UINT32_MAX, true };
    BindKey pathTracerMainColorKey = { ShaderType::kLibrary, ViewType::kRWTexture, 0, 0, 1, UINT32_MAX };
    pathTracerBindingSetLayout = RenderUtils::CreateLayoutSet(device, *camera, { drawRootConstant, pathTracerMainColorKey, Scene::accelerationStructureKey }, RenderUtils::All, RenderUtils::Compute);
    pathTracerBindingSet = RenderUtils::CreateBindingSet(device, pathTracerBindingSetLayout, *camera, { { drawRootConstant, nullptr }, { pathTracerMainColorKey, pathTracingAccumulationView }, Scene::accelerationStructureBinding }, RenderUtils::All, RenderUtils::Compute);

    //Create HW path tracing pipeline
    std::vector<RayTracingShaderGroup> groups;
    groups.push_back({ RayTracingShaderGroupType::kGeneral, pathTracingLibrary->GetId("RayGen") });
    groups.push_back({ RayTracingShaderGroupType::kGeneral, pathTracingMissLibrary->GetId("Miss") });
    groups.push_back({ RayTracingShaderGroupType::kTrianglesHitGroup, 0, pathTracingHitLibrary->GetId("Hit") });
    pathTracerPipeline = device->CreateRayTracingPipeline({ pathTracingProgram, pathTracerBindingSetLayout, groups });

    //Create the shader table for HW path tracing
    std::shared_ptr<Resource> shaderTable =
        device->CreateBuffer(BindFlag::kShaderTable, device->GetShaderTableAlignment() * groups.size());
    shaderTable->CommitMemory(MemoryType::kUpload);
    shaderTable->SetName("Shader Table");

    decltype(auto) shaderHandles = pathTracerPipeline->GetRayTracingShaderGroupHandles(0, groups.size());
    for (size_t i = 0; i < groups.size(); ++i) {
        shaderTable->UpdateUploadBuffer(i * device->GetShaderTableAlignment(),
            shaderHandles.data() + i * device->GetShaderGroupHandleSize(),
            device->GetShaderGroupHandleSize());
    }

    shaderTables.raygen = {
        shaderTable,
        0 * device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
    };
    shaderTables.miss = {
        shaderTable,
        1 * device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
    };
    shaderTables.hit = {
        shaderTable,
        2 * device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
    };

    BindKey pathTracerAccumulationMainColorKey = { ShaderType::kCompute, ViewType::kTexture, 0, 0 };
    BindKey mainColorKey = { ShaderType::kCompute, ViewType::kRWTexture, 0, 0 };
    pathTracerResolveBindingSetLayout = RenderUtils::CreateLayoutSet(device, *camera, { pathTracerAccumulationMainColorKey, mainColorKey }, RenderUtils::All, RenderUtils::Compute);
    pathTracerResolveBindingSet = RenderUtils::CreateBindingSet(device, pathTracerResolveBindingSetLayout, *camera, { { pathTracerAccumulationMainColorKey, pathTracingAccumulationView }, { mainColorKey, mainColorTextureView } }, RenderUtils::All, RenderUtils::Compute);
    pathTracingResolve = RenderUtils::CreateComputePipeline(device, "shaders/PathTracingResolve.hlsl", "main", pathTracerResolveBindingSetLayout);

    BindKey pathTracerClearMainColorKey = { ShaderType::kCompute, ViewType::kRWTexture, 0, 0 };
    pathTracerClearBindingSetLayout = RenderUtils::CreateLayoutSet(device, *camera, { pathTracerClearMainColorKey }, RenderUtils::All, RenderUtils::Compute);
    pathTracerClearBindingSet = RenderUtils::CreateBindingSet(device, pathTracerClearBindingSetLayout, *camera, { { pathTracerClearMainColorKey, pathTracingAccumulationView } }, RenderUtils::All, RenderUtils::Compute);
    pathTracingClear = RenderUtils::CreateComputePipeline(device, "shaders/PathTracingResolve.hlsl", "clear", pathTracerClearBindingSetLayout);

    vec2BlueNoiseTexture = Texture::Create3D(device, MODERN_RENDERER_ASSETS_PATH "STBN/stbn_vec2_2Dx1D_128x128x64");
}

void Renderer::Controls::OnKey(int key, int action)
{
	if (key == GLFW_KEY_SPACE)
	{
        if (action == GLFW_PRESS)
        {
            rendererMode = (RendererMode)!(bool)rendererMode;
        }
	}

    if (key == GLFW_KEY_F11 || key == GLFW_KEY_F12)
    {
        if (action == GLFW_PRESS)
            RenderDoc::EnqueueCaptureNextFrame();
    }
}

void Renderer::UpdateCommandList(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    cmd->Reset();
    cmd->BeginEvent("RenderFrame");

    cmd->SetViewport(0, 0, appSize.width(), appSize.height());
    cmd->SetScissorRect(0, 0, appSize.width(), appSize.height());

    Profiler::BeginFrame();
    Profiler::BeginMarker(cmd, "Total Frame");

    if (camera.HasMoved())
        resetPathTracingAccumulation = true;

	if (controls.rendererMode == RendererMode::Rasterization)
	{
		RenderRasterization(cmd, backBuffer, camera, scene);
	}
	else
	{
		RenderPathTracing(cmd, backBuffer, camera, scene);
	}

    if (!RenderSettings::noUI)
    {
        // Render ImGUI on top of the scene
        cmd->ResourceBarrier({ { mainColorTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
        cmd->ResourceBarrier({ { mainDepthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite } });
        cmd->BeginRenderPass(imGUIPass, imGUIFrameBuffer, {});
        imGUI->OnRender(cmd);
        cmd->EndRenderPass();
        cmd->ResourceBarrier({ { mainColorTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
        cmd->ResourceBarrier({ { mainDepthTexture, ResourceState::kDepthStencilWrite, ResourceState::kCommon } });
    }

    // Copy final image to the backbuffer
    // TODO: tonemap color buffer to LDR backbuffer
    cmd->ResourceBarrier({ { backBuffer, ResourceState::kPresent, ResourceState::kCopyDest } });
    cmd->ResourceBarrier({ { mainColorTexture, ResourceState::kCommon, ResourceState::kCopySource } });
    cmd->CopyTexture(mainColorTexture, backBuffer, { { appSize.width(), appSize.height(), 1 } });
    cmd->ResourceBarrier({ { backBuffer, ResourceState::kCopyDest, ResourceState::kPresent } });
    cmd->ResourceBarrier({ { mainColorTexture, ResourceState::kCopySource, ResourceState::kCommon } });

    Profiler::EndMarker(cmd);
    Profiler::EndFrame(cmd);

    cmd->EndEvent();
    cmd->Close();
}

void Renderer::RenderRasterization(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    renderPipeline->Render(cmd, backBuffer, scene);
}

void Renderer::RenderPathTracing(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    DXCommandList* dxCmd = (DXCommandList*)cmd.get();
    if (resetPathTracingAccumulation)
    {
        resetPathTracingAccumulation = false;
        pathTracingFrameIndex = 0;
		cmd->ResourceBarrier({ { pathTracingAccumulationTexture, ResourceState::kCommon, ResourceState::kUnorderedAccess } });
		cmd->BindPipeline(pathTracingClear.pipeline);
		cmd->BindBindingSet(pathTracerClearBindingSet);
        cmd->Dispatch(appSize.width() / 8, appSize.height() / 8, 1);
        cmd->ResourceBarrier({ { pathTracingAccumulationTexture, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
	}

    cmd->ResourceBarrier({ { pathTracingAccumulationTexture, ResourceState::kCommon, ResourceState::kUnorderedAccess } });
    cmd->BindPipeline(pathTracerPipeline);
    cmd->BindBindingSet(pathTracerBindingSet);
    for (int i = 0; i < RenderSettings::integrationCountPerFrame; i++)
    {
        dxCmd->SetComputeConstant(0, pathTracingFrameIndex++, 0);
        cmd->DispatchRays(shaderTables, appSize.width(), appSize.height(), 1);
    }
    cmd->ResourceBarrier({ { pathTracingAccumulationTexture, ResourceState::kUnorderedAccess, ResourceState::kCommon } });

    cmd->ResourceBarrier({ { mainColorTexture, ResourceState::kCommon, ResourceState::kUnorderedAccess } });
    cmd->ResourceBarrier({ { pathTracingAccumulationTexture, ResourceState::kCommon, ResourceState::kGenericRead } });
    cmd->BindPipeline(pathTracingResolve.pipeline);
    cmd->BindBindingSet(pathTracerResolveBindingSet);
    cmd->Dispatch(appSize.width() / 8, appSize.height() / 8, 1);
    cmd->ResourceBarrier({ { pathTracingAccumulationTexture, ResourceState::kGenericRead, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { mainColorTexture, ResourceState::kUnorderedAccess, ResourceState::kCommon } });
}
