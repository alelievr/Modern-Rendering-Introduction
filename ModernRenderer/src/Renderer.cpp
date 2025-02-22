#include "Renderer.hpp"
#include "RenderDoc.hpp"
#include <CommandList/DXCommandList.h>
#include "RenderUtils.hpp"

Renderer::Renderer(std::shared_ptr<Device> device, AppBox& app, Camera& camera)
{
    this->device = device;
    this->appSize = app.GetAppSize();
    this->camera = &camera;

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

    // Create the framebuffer
    ViewDesc stencil2DDesc = {};
    stencil2DDesc.view_type = ViewType::kDepthStencil;
    stencil2DDesc.dimension = ViewDimension::kTexture2D;

    FramebufferDesc desc = {};
    desc.render_pass = loadStoreColorRenderPass;
    desc.width = appSize.width();
    desc.height = appSize.height();
    desc.colors = { mainColorRenderTargetView };
    desc.depth_stencil = mainDepthTextureView;
    mainColorFrameBuffer = device->CreateFramebuffer(desc);
}

void Renderer::CompileShaders()
{
    // Create mesh shader program
    std::shared_ptr<Shader> pixelMeshshader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/PixelShader.hlsl", "main", ShaderType::kPixel, "6_5" });
    std::shared_ptr<Shader> meshShader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/MeshShader.hlsl", "main", ShaderType::kMesh, "6_5" });
    meshShaderProgram = device->CreateProgram({ meshShader, pixelMeshshader });

    // Create HW path tracing program
    pathTracingLibrary = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/RayTracing.hlsl", "", ShaderType::kLibrary, "6_5" });
    pathTracingHitLibrary = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/RayTracingHit.hlsl", "", ShaderType::kLibrary, "6_5" });
    pathTracingCallableLibrary = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/RayTracingCallable.hlsl", "", ShaderType::kLibrary, "6_5" });
    pathTracingProgram = device->CreateProgram({ pathTracingLibrary, pathTracingHitLibrary, pathTracingCallableLibrary });
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

    BindKey pathTracerMainColorKey = { ShaderType::kLibrary, ViewType::kRWTexture, 0, 0, 1, UINT32_MAX };
    std::shared_ptr<BindingSetLayout> pathTracerLayout = device->CreateBindingSetLayout({ camera->cameraDataKeyCompute, pathTracerMainColorKey, Scene::accelerationStructureKey });
    pathTracerBindingSet = device->CreateBindingSet(pathTracerLayout);
    pathTracerBindingSet->WriteBindings({ camera->cameraDataDescCompute, { pathTracerMainColorKey, mainColorTextureView }, Scene::accelerationStructureBinding });

    // Create HW path tracing pipeline
    std::vector<RayTracingShaderGroup> groups;
    groups.push_back({ RayTracingShaderGroupType::kGeneral, pathTracingLibrary->GetId("ray_gen") });
    groups.push_back({ RayTracingShaderGroupType::kGeneral, pathTracingLibrary->GetId("miss") });
    groups.push_back({ RayTracingShaderGroupType::kTrianglesHitGroup, 0, pathTracingHitLibrary->GetId("Hit") });
    groups.push_back({ RayTracingShaderGroupType::kTrianglesHitGroup, 0, pathTracingHitLibrary->GetId("closest_green") });
    groups.push_back({ RayTracingShaderGroupType::kGeneral, pathTracingCallableLibrary->GetId("callable") });
    pathTracerPipeline = device->CreateRayTracingPipeline({ pathTracingProgram, pathTracerLayout, groups });

    // Create the shader table for HW path tracing
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
        2 * device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
    };
    shaderTables.callable = {
        shaderTable,
        4 * device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
        device->GetShaderTableAlignment(),
    };
}

void Renderer::Controls::OnKey(int key, int action)
{
	if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS)
			rendererMode = (RendererMode)!(bool)rendererMode;
	}

    if (key == GLFW_KEY_F11 || key == GLFW_KEY_F12)
    {
        if (action == GLFW_PRESS)
            RenderDoc::EnqueueCaptureNextFrame();
    }
}

void Renderer::UpdateCommandList(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    commandList->Reset();
    commandList->BeginEvent("RenderFrame");

    commandList->SetViewport(0, 0, appSize.width(), appSize.height());
    commandList->SetScissorRect(0, 0, appSize.width(), appSize.height());

	if (controls.rendererMode == RendererMode::Rasterization)
	{
		RenderRasterization(commandList, backBuffer, camera, scene);
	}
	else
	{
		RenderPathTracing(commandList, backBuffer, camera, scene);
	}

    // Copy final image to the backbuffer
    // TODO: tonemap color buffer to LDR backbuffer
    commandList->ResourceBarrier({ { backBuffer, ResourceState::kPresent, ResourceState::kCopyDest } });
    commandList->ResourceBarrier({ { mainColorTexture, ResourceState::kCommon, ResourceState::kCopySource } });
    commandList->CopyTexture(mainColorTexture, backBuffer, { { appSize.width(), appSize.height(), 1 } });
    commandList->ResourceBarrier({ { backBuffer, ResourceState::kCopyDest, ResourceState::kPresent } });
    commandList->ResourceBarrier({ { mainColorTexture, ResourceState::kCopySource, ResourceState::kCommon } });

    commandList->EndEvent();
    commandList->Close();
}

void Renderer::RenderRasterization(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    renderPipeline->Render(commandList, backBuffer, scene);
}

void Renderer::RenderPathTracing(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    commandList->BindPipeline(pathTracerPipeline);
    commandList->BindBindingSet(pathTracerBindingSet);
    commandList->DispatchRays(shaderTables, appSize.width(), appSize.height(), 1);
}
