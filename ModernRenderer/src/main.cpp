#include "AppBox/AppBox.h"
#include "AppSettings/ArgsParser.h"
#include "Instance/Instance.h"
#include "Camera.hpp"

int main(int argc, char* argv[])
{
    Settings settings = ParseArgs(argc, argv);
    AppBox app("ModernRenderer", settings);
    AppSize appSize = app.GetAppSize();

    glfwSetInputMode(app.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::shared_ptr<Instance> instance = CreateInstance(settings.api_type);
    std::shared_ptr<Adapter> adapter = std::move(instance->EnumerateAdapters()[settings.required_gpu_index]);
    app.SetGpuName(adapter->GetName());
    std::shared_ptr<Device> device = adapter->CreateDevice();
    std::shared_ptr<CommandQueue> command_queue = device->GetCommandQueue(CommandListType::kGraphics);
    constexpr uint32_t frame_count = 2;
    std::shared_ptr<Swapchain> swapchain = device->CreateSwapchain(app.GetNativeWindow(), appSize.width(),
                                                                   appSize.height(), frame_count, settings.vsync);
    uint64_t fence_value = 0;
    std::shared_ptr<Fence> fence = device->CreateFence(fence_value);

    std::vector<uint32_t> index_data = { 0, 1, 2 };
    std::shared_ptr<Resource> index_buffer =
        device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * index_data.size());
    index_buffer->CommitMemory(MemoryType::kUpload);
    index_buffer->UpdateUploadBuffer(0, index_data.data(), sizeof(index_data.front()) * index_data.size());

    std::shared_ptr<Resource> outputTexture = device->CreateTexture(TextureType::k2D, BindFlag::kRenderTarget | BindFlag::kUnorderedAccess | BindFlag::kShaderResource | BindFlag::kCopySource, gli::format::FORMAT_RGBA8_UNORM_PACK8, 1, appSize.width(), appSize.height(), 1, 1);
    outputTexture->CommitMemory(MemoryType::kDefault);
    outputTexture->SetName("ColorTexture");
    ViewDesc outputTextureViewDesc = {};
    outputTextureViewDesc.view_type = ViewType::kRWTexture;
    outputTextureViewDesc.dimension = ViewDimension::kTexture2D;
    std::shared_ptr<View> outputTextureView = device->CreateView(outputTexture, outputTextureViewDesc);

    std::vector<glm::vec3> vertex_data = {
        glm::vec3(-0.5, -0.5, 0.0),
        glm::vec3(0.0, 0.5, 0.0),
        glm::vec3(0.5, -0.5, 0.0),
    };
    std::shared_ptr<Resource> vertex_buffer = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest,
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
        { MODERN_RENDERER_ASSETS_PATH "shaders/Compute.hlsl", "main", ShaderType::kCompute, "6_5" });
    std::shared_ptr<Program> compute_program = device->CreateProgram({ compute_test });

    Camera camera = Camera(device, app);

    ViewDesc constant_view_desc = {};
    constant_view_desc.view_type = ViewType::kConstantBuffer;
    constant_view_desc.dimension = ViewDimension::kBuffer;
    std::shared_ptr<View> constant_view = device->CreateView(constant_buffer, constant_view_desc);
    BindKey settings_key = { ShaderType::kPixel, ViewType::kConstantBuffer, 1, 0, 1, UINT32_MAX };
    std::shared_ptr<BindingSetLayout> layout = device->CreateBindingSetLayout({ camera.cameraDataKeyVertex, settings_key });
    std::shared_ptr<BindingSet> binding_set = device->CreateBindingSet(layout);
    binding_set->WriteBindings({ camera.cameraDataDescVertex, { settings_key, constant_view } });
    BindKey compute_settings_key = { ShaderType::kCompute, ViewType::kRWTexture, 0, 0, 1, UINT32_MAX };
    std::shared_ptr<BindingSetLayout> compute_layout = device->CreateBindingSetLayout({ camera.cameraDataKeyCompute, compute_settings_key });
    std::shared_ptr<BindingSet> compute_binding_set = device->CreateBindingSet(compute_layout);

    RenderPassDesc render_pass_desc = {
        { { swapchain->GetFormat(), RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore } },
    };
    std::shared_ptr<RenderPass> render_pass = device->CreateRenderPass(render_pass_desc);
    ClearDesc clear_desc = { { { 0.0, 0.2, 0.4, 1.0 } } };
    GraphicsPipelineDesc pipeline_desc = {
        program,
        layout,
        { { 0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(vertex_data.front()) } },
        render_pass,
    };
    pipeline_desc.rasterizer_desc = { FillMode::kSolid, CullMode::kNone, 0 };
    std::shared_ptr<Pipeline> pipeline = device->CreateGraphicsPipeline(pipeline_desc);

    ComputePipelineDesc compute_desc = {
        compute_program,
        compute_layout,
    };
    std::shared_ptr<Pipeline> compute_pipeline = device->CreateComputePipeline(compute_desc);

    // Create GFX Buffer

    std::array<uint64_t, frame_count> fence_values = {};
    std::vector<std::shared_ptr<CommandList>> command_lists;
    std::vector<std::shared_ptr<Framebuffer>> framebuffers;
    for (uint32_t i = 0; i < frame_count; ++i) {
        ViewDesc back_buffer_view_desc = {};
        back_buffer_view_desc.view_type = ViewType::kRenderTarget;
        back_buffer_view_desc.dimension = ViewDimension::kTexture2D;
        std::shared_ptr<Resource> back_buffer = swapchain->GetBackBuffer(i);
        std::shared_ptr<View> back_buffer_view = device->CreateView(back_buffer, back_buffer_view_desc);
        FramebufferDesc framebuffer_desc = {};
        framebuffer_desc.render_pass = render_pass;
        framebuffer_desc.width = appSize.width();
        framebuffer_desc.height = appSize.height();
        framebuffer_desc.colors = { back_buffer_view };
        std::shared_ptr<Framebuffer> framebuffer =
            framebuffers.emplace_back(device->CreateFramebuffer(framebuffer_desc));
        std::shared_ptr<CommandList> command_list =
            command_lists.emplace_back(device->CreateCommandList(CommandListType::kGraphics));

        compute_binding_set->WriteBindings({ camera.cameraDataDescCompute, { compute_settings_key, outputTextureView } });

        FramebufferDesc framebuffer_desc2 = {};
        framebuffer_desc2.render_pass = render_pass;
        framebuffer_desc2.width = appSize.width();
        framebuffer_desc2.height = appSize.height();
        framebuffer_desc2.colors = { device->CreateView(outputTexture, back_buffer_view_desc) };
        std::shared_ptr<Framebuffer> framebufferIntermediate =
            framebuffers.emplace_back(device->CreateFramebuffer(framebuffer_desc2));

        // Dispatch compute to clear RT
        command_list->BindPipeline(compute_pipeline);
        command_list->BindBindingSet(compute_binding_set);
        command_list->Dispatch(appSize.width() / 8, appSize.height() / 8, 1);

        command_list->BindPipeline(pipeline);
        command_list->BindBindingSet(binding_set);
        command_list->SetViewport(0, 0, appSize.width(), appSize.height());
        command_list->SetScissorRect(0, 0, appSize.width(), appSize.height());
        command_list->IASetIndexBuffer(index_buffer, gli::format::FORMAT_R32_UINT_PACK32);
        command_list->IASetVertexBuffer(0, vertex_buffer);
        command_list->ResourceBarrier({ { outputTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
        command_list->BeginRenderPass(render_pass, framebufferIntermediate, clear_desc);
        command_list->DrawIndexed(3, 1, 0, 0, 0);
        command_list->EndRenderPass();
        command_list->ResourceBarrier({ { outputTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });

        // Copy compute result to backbuffer
        command_list->ResourceBarrier({ { back_buffer, ResourceState::kPresent, ResourceState::kCopyDest } });
        command_list->ResourceBarrier({ { outputTexture, ResourceState::kCommon, ResourceState::kCopySource } });
        command_list->CopyTexture(outputTexture, back_buffer, { { appSize.width(), appSize.height(), 1 } });
        command_list->ResourceBarrier({ { back_buffer, ResourceState::kCopyDest, ResourceState::kPresent } });
        command_list->ResourceBarrier({ { outputTexture, ResourceState::kCopySource, ResourceState::kCommon } });

        // TODO: copy color buffer to backbuffer

        command_list->Close();
    }

    while (!app.PollEvents())
    {
        uint32_t frame_index = swapchain->NextImage(fence, ++fence_value);
        command_queue->Wait(fence, fence_value);
        fence->Wait(fence_values[frame_index]);
        //RenderFrame(command_queue);
        
        // Update camera controls and GPU buffer
        camera.UpdateCamera(appSize);
        
        // Then execute the rendering commands on the GPU.
        command_queue->ExecuteCommandLists({ command_lists[frame_index] });

        command_queue->Signal(fence, fence_values[frame_index] = ++fence_value);
        swapchain->Present(fence, fence_values[frame_index]);
    }
    command_queue->Signal(fence, ++fence_value);
    fence->Wait(fence_value);
    return 0;
}
