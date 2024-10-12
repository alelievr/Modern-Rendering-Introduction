#include "Renderer.hpp"

Renderer::Renderer(std::shared_ptr<Device> device, AppBox& app, Camera& camera)
{
    this->device = device;
    this->appSize = app.GetAppSize();

	app.SubscribeEvents((InputEvents*)&controls, nullptr);

    // TODO: split this big block of commands

	std::vector<uint32_t> index_data = { 0, 1, 2 };
	index_buffer = device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * index_data.size());
	index_buffer->CommitMemory(MemoryType::kUpload);
	index_buffer->UpdateUploadBuffer(0, index_data.data(), sizeof(index_data.front()) * index_data.size());

    mainColorTexture = device->CreateTexture(TextureType::k2D, BindFlag::kRenderTarget | BindFlag::kUnorderedAccess | BindFlag::kShaderResource | BindFlag::kCopySource, gli::format::FORMAT_RGBA8_UNORM_PACK8, 1, appSize.width(), appSize.height(), 1, 1);
    mainColorTexture->CommitMemory(MemoryType::kDefault);
    mainColorTexture->SetName("ColorTexture");
    ViewDesc outputTextureViewDesc = {};
    outputTextureViewDesc.view_type = ViewType::kRWTexture;
    outputTextureViewDesc.dimension = ViewDimension::kTexture2D;
    std::shared_ptr<View> outputTextureView = device->CreateView(mainColorTexture, outputTextureViewDesc);

    std::vector<glm::vec3> vertex_data = {
        glm::vec3(-0.5, -0.5, 0.0),
        glm::vec3(0.0, 0.5, 0.0),
        glm::vec3(0.5, -0.5, 0.0),
    };
    vertex_buffer = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest,
        sizeof(vertex_data.front()) * vertex_data.size());
    vertex_buffer->CommitMemory(MemoryType::kUpload);
    vertex_buffer->UpdateUploadBuffer(0, vertex_data.data(), sizeof(vertex_data.front()) * vertex_data.size());

    // Object CBuffer
    glm::vec4 constant_data = glm::vec4(1, 0, 0, 1);
    std::shared_ptr<Resource> constant_buffer =
        device->CreateBuffer(BindFlag::kConstantBuffer | BindFlag::kCopyDest, sizeof(constant_data));
    constant_buffer->CommitMemory(MemoryType::kUpload);
    constant_buffer->UpdateUploadBuffer(0, &constant_data, sizeof(constant_data));

    std::shared_ptr<Shader> vertex_shader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/VertexShader.hlsl", "main", ShaderType::kVertex, "6_5" });
    std::shared_ptr<Shader> pixel_shader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/PixelShader.hlsl", "main", ShaderType::kPixel, "6_5" });
    std::shared_ptr<Program> program = device->CreateProgram({ vertex_shader, pixel_shader });

    std::shared_ptr<Shader> compute_test = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/PathTracer.hlsl", "main", ShaderType::kCompute, "6_5" });
    std::shared_ptr<Program> compute_program = device->CreateProgram({ compute_test });

    ViewDesc constant_view_desc = {};
    constant_view_desc.view_type = ViewType::kConstantBuffer;
    constant_view_desc.dimension = ViewDimension::kBuffer;
    std::shared_ptr<View> constant_view = device->CreateView(constant_buffer, constant_view_desc);
    BindKey settings_key = { ShaderType::kPixel, ViewType::kConstantBuffer, 1, 0, 1, UINT32_MAX };
    std::shared_ptr<BindingSetLayout> layout = device->CreateBindingSetLayout({ camera.cameraDataKeyVertex, settings_key });
    objectBindingSet = device->CreateBindingSet(layout);
    objectBindingSet->WriteBindings({ camera.cameraDataDescVertex, { settings_key, constant_view } });
    BindKey compute_settings_key = { ShaderType::kCompute, ViewType::kRWTexture, 0, 0, 1, UINT32_MAX };
    std::shared_ptr<BindingSetLayout> compute_layout = device->CreateBindingSetLayout({ camera.cameraDataKeyCompute, compute_settings_key });
    pathTracerBindingSet = device->CreateBindingSet(compute_layout);

    RenderPassDesc render_pass_desc = {
        { { gli::FORMAT_RGBA8_UNORM_PACK8, RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore } },
    };
    loadStoreColorRenderPass = device->CreateRenderPass(render_pass_desc);
    render_pass_desc = {
    { { gli::FORMAT_RGBA8_UNORM_PACK8, RenderPassLoadOp::kClear, RenderPassStoreOp::kStore } },
    };
    clearColorRenderPass = device->CreateRenderPass(render_pass_desc);

    ClearDesc clear_desc = { { { 0.0, 0.2, 0.4, 1.0 } } };
    GraphicsPipelineDesc pipeline_desc = {
        program,
        layout,
        { { 0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(vertex_data.front()) } },
        clearColorRenderPass,
    };
    pipeline_desc.rasterizer_desc = { FillMode::kSolid, CullMode::kNone, 0 };
    objectPipeline = device->CreateGraphicsPipeline(pipeline_desc);

    ComputePipelineDesc compute_desc = {
        compute_program,
        compute_layout,
    };
    pathTracerPipeline = device->CreateComputePipeline(compute_desc);

    pathTracerBindingSet->WriteBindings({ camera.cameraDataDescCompute, { compute_settings_key, outputTextureView } });

    ViewDesc renderTarget2DDesc = {};
    renderTarget2DDesc.view_type = ViewType::kRenderTarget;
    renderTarget2DDesc.dimension = ViewDimension::kTexture2D;

    FramebufferDesc desc = {};
    desc.render_pass = loadStoreColorRenderPass;
    desc.width = appSize.width();
    desc.height = appSize.height();
    desc.colors = { device->CreateView(mainColorTexture, renderTarget2DDesc) };
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
}

void Renderer::UpdateCommandList(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, Camera camera, Scene scene)
{
    commandList->Reset();
    commandList->BeginEvent("RenderFrame");
	if (controls.GetRendererMode() == RendererMode::Rasterization)
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

void Renderer::RenderRasterization(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, Camera camera, Scene scene)
{
    // TODO: clear render pass

    ClearDesc clear_desc = { { { 0.0, 0.2, 0.4, 1.0 } } }; // Clear Color
    commandList->BindPipeline(objectPipeline);
    commandList->BindBindingSet(objectBindingSet);
    commandList->SetViewport(0, 0, appSize.width(), appSize.height());
    commandList->SetScissorRect(0, 0, appSize.width(), appSize.height());
    commandList->IASetIndexBuffer(index_buffer, gli::format::FORMAT_R32_UINT_PACK32);
    commandList->IASetVertexBuffer(0, vertex_buffer);
    commandList->ResourceBarrier({ { mainColorTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    commandList->BeginRenderPass(clearColorRenderPass, mainColorFrameBuffer, clear_desc);
    commandList->DrawIndexed(3, 1, 0, 0, 0);
    commandList->EndRenderPass();
    commandList->ResourceBarrier({ { mainColorTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
}

void Renderer::RenderPathTracing(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, Camera camera, Scene)
{
    // Dispatch compute to clear RT
    commandList->BindPipeline(pathTracerPipeline);
    commandList->BindBindingSet(pathTracerBindingSet);
    commandList->Dispatch(appSize.width() / 8, appSize.height() / 8, 1);
}
