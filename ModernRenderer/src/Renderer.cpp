#include "Renderer.hpp"
#include "RenderDoc.hpp"
#include <CommandList/DXCommandList.h>
#include "RenderUtils.hpp"

struct DrawRootConstants
{
	uint32_t materialIndex;
};

Renderer::Renderer(std::shared_ptr<Device> device, AppBox& app, Camera& camera)
{
    this->device = device;
    this->appSize = app.GetAppSize();

	app.SubscribeEvents((InputEvents*)&controls, nullptr);

    // TODO: split this big block of commands

    mainColorTexture = device->CreateTexture(TextureType::k2D, BindFlag::kRenderTarget | BindFlag::kUnorderedAccess | BindFlag::kShaderResource | BindFlag::kCopySource, gli::format::FORMAT_RGBA8_UNORM_PACK8, 1, appSize.width(), appSize.height(), 1, 1);
    mainColorTexture->CommitMemory(MemoryType::kDefault);
    mainColorTexture->SetName("ColorTexture");
    ViewDesc outputTextureViewDesc = {};
    outputTextureViewDesc.view_type = ViewType::kRWTexture;
    outputTextureViewDesc.dimension = ViewDimension::kTexture2D;
    std::shared_ptr<View> mainColorTextureView = device->CreateView(mainColorTexture, outputTextureViewDesc);

    mainDepthTexture = device->CreateTexture(TextureType::k2D, BindFlag::kDepthStencil | BindFlag::kShaderResource, gli::format::FORMAT_D32_SFLOAT_S8_UINT_PACK64, 1, appSize.width(), appSize.height(), 1, 1);
    mainDepthTexture->CommitMemory(MemoryType::kDefault);
    mainDepthTexture->SetName("DepthTexture");
    //std::shared_ptr<View> mainDepthTextureView = device->CreateView(mainDepthTexture, outputTextureViewDesc);

    // Standard vertex pipeline: TODO: remove it once mesh path works
    std::shared_ptr<Shader> vertex_shader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/VertexShader.hlsl", "main", ShaderType::kVertex, "6_5" });
    std::shared_ptr<Shader> pixel_shader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/PixelShaderFromVertex.hlsl", "main", ShaderType::kPixel, "6_5" });
    std::shared_ptr<Program> program = device->CreateProgram({ vertex_shader, pixel_shader });

    // Create mesh shader program
    std::shared_ptr<Shader> pixelMeshshader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/PixelShader.hlsl", "main", ShaderType::kPixel, "6_5" });
    std::shared_ptr<Shader> meshShader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/MeshShader.hlsl", "main", ShaderType::kMesh, "6_5" });
    std::shared_ptr<Program> programMesh = device->CreateProgram({ meshShader, pixelMeshshader });

    std::shared_ptr<Shader> compute_test = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/PathTracerScene0.hlsl", "main", ShaderType::kCompute, "6_5" });
    std::shared_ptr<Program> compute_program = device->CreateProgram({ compute_test });

    // Compute stage allows to bind to every shader stages
    BindKey drawRootConstant = { ShaderType::kCompute, ViewType::kConstantBuffer, 1, 0, 2, UINT32_MAX, true };
    std::shared_ptr<BindingSetLayout> layout = RenderUtils::CreateLayoutSet(device, camera, { drawRootConstant });
    std::shared_ptr<BindingSetLayout> meshShaderLayout = RenderUtils::CreateLayoutSet(device, camera, { drawRootConstant });
    objectBindingSet = RenderUtils::CreateBindingSet(device, layout, camera, { { drawRootConstant, nullptr } });
    
    BindKey pathTracerMainColorKey = { ShaderType::kCompute, ViewType::kRWTexture, 0, 0, 1, UINT32_MAX };
    std::shared_ptr<BindingSetLayout> compute_layout = device->CreateBindingSetLayout({ camera.cameraDataKeyCompute, pathTracerMainColorKey });
    pathTracerBindingSet = device->CreateBindingSet(compute_layout);
    pathTracerBindingSet->WriteBindings({ camera.cameraDataDescCompute, { pathTracerMainColorKey, mainColorTextureView } });

	// Create the load/store render pass setup
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

    GraphicsPipelineDesc pipelineDesc = {
        program,
        layout,
        { Mesh::GetInputAssemblerLayout() },
        clearColorRenderPass,
    };
    pipelineDesc.rasterizer_desc = { FillMode::kSolid, CullMode::kBack, 0 };
    objectPipeline = device->CreateGraphicsPipeline(pipelineDesc);

    GraphicsPipelineDesc meshShaderPipelineDesc = {
        programMesh,
        layout,
        {},
        clearColorRenderPass,
    };
    meshShaderPipelineDesc.rasterizer_desc = { FillMode::kSolid, CullMode::kBack, 0 };

    objectMeshShaderPipeline = device->CreateGraphicsPipeline(meshShaderPipelineDesc);

    ComputePipelineDesc computeDesc = {
        compute_program,
        compute_layout,
    };
    pathTracerPipeline = device->CreateComputePipeline(computeDesc);

    ViewDesc renderTarget2DDesc = {};
    renderTarget2DDesc.view_type = ViewType::kRenderTarget;
    renderTarget2DDesc.dimension = ViewDimension::kTexture2D;
    ViewDesc stencil2DDesc = {};
    stencil2DDesc.view_type = ViewType::kDepthStencil;
    stencil2DDesc.dimension = ViewDimension::kTexture2D;

    FramebufferDesc desc = {};
    desc.render_pass = loadStoreColorRenderPass;
    desc.width = appSize.width();
    desc.height = appSize.height();
    desc.colors = { device->CreateView(mainColorTexture, renderTarget2DDesc) };
	desc.depth_stencil = device->CreateView(mainDepthTexture, stencil2DDesc);
    mainColorFrameBuffer = device->CreateFramebuffer(desc);
}

Renderer::~Renderer()
{
}

void Renderer::Controls::OnKey(int key, int action)
{
	if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS)
			rendererMode = (RendererMode)!(bool)rendererMode;
	}

    if (key == GLFW_KEY_F12)
    {
        if (action == GLFW_PRESS)
            RenderDoc::EnqueueCaptureNextFrame();
    }
}

void Renderer::UpdateCommandList(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    commandList->Reset();
    commandList->BeginEvent("RenderFrame");

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

void DrawScene(std::shared_ptr<CommandList> commandList, std::shared_ptr<Scene> scene)
{
    // Get the native command list to push constants directly to the commnand buffer.
    // This allows to have a unique index per draw without having to bind a new constant buffer.
    auto dxCommandList = (DXCommandList*)commandList.get();

    // TODO: batch per materials to avoid pipeline switches & reduce CBuffer updates
    for (auto& instance : scene->instances)
    {
        dxCommandList->SetGraphicsConstant(0, instance.instanceDataOffset, 2);

        for (auto& r : instance.model.parts)
		{
            // TODO: mesh index when meshlets are supported
            int materialIndex = r.material->materialIndex;

            // Bind per-draw data, we only need an index, the rest is bindless
            dxCommandList->SetGraphicsConstant(0, materialIndex, 0);
            dxCommandList->SetGraphicsConstant(0, r.mesh.meshletOffset, 1);

            commandList->DispatchMesh(r.mesh.meshletCount);
		}
    }
}

void Renderer::RenderRasterization(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    // TODO: clear render pass

    ClearDesc clear_desc = { { { 0.0, 0.2, 0.4, 1.0 } } }; // Clear Color
    commandList->BindPipeline(objectMeshShaderPipeline);
    commandList->BindBindingSet(objectBindingSet);
    commandList->SetViewport(0, 0, appSize.width(), appSize.height());
    commandList->SetScissorRect(0, 0, appSize.width(), appSize.height());
    commandList->ResourceBarrier({ { mainColorTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    commandList->ResourceBarrier({ { mainDepthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite } });
    commandList->BeginRenderPass(clearColorRenderPass, mainColorFrameBuffer, clear_desc);

    DrawScene(commandList, scene);

    commandList->EndRenderPass();
    commandList->ResourceBarrier({ { mainColorTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
    commandList->ResourceBarrier({ { mainDepthTexture, ResourceState::kDepthStencilWrite, ResourceState::kCommon } });
}

void Renderer::RenderPathTracing(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene)
{
    // Dispatch compute to clear RT
    commandList->BindPipeline(pathTracerPipeline);
    commandList->BindBindingSet(pathTracerBindingSet);
    commandList->Dispatch(appSize.width() / 8, appSize.height() / 8, 1);
}
