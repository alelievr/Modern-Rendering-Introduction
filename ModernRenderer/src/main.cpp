#include "AppBox/AppBox.h"
#include "AppSettings/ArgsParser.h"
#include "Instance/Instance.h"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "InputController.hpp"
#include "RenderDoc.hpp"
#include "ScreenShotController.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

//#define LOAD_RENDERDOC

int main(int argc, char* argv[])
{
    Settings settings = ParseArgs(argc, argv);
    AppBox app("ModernRenderer", settings);
    AppSize appSize = app.GetAppSize();

#if defined(LOAD_RENDERDOC)
    // Loading renderdoc will diisable the validation layer, make sure there is no error before enabling it.
    RenderDoc::LoadRenderDoc();
#endif

    glfwSetInputMode(app.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::shared_ptr<Instance> instance = CreateInstance(settings.api_type);
    std::shared_ptr<Adapter> adapter = std::move(instance->EnumerateAdapters()[settings.required_gpu_index]);
    app.SetGpuName(adapter->GetName());
    std::shared_ptr<Device> device = adapter->CreateDevice();
    std::shared_ptr<CommandQueue> command_queue = device->GetCommandQueue(CommandListType::kGraphics);
    constexpr uint32_t swapchainTextureCount = 2;
    std::shared_ptr<Swapchain> swapchain = device->CreateSwapchain(app.GetNativeWindow(), appSize.width(),
                                                                   appSize.height(), swapchainTextureCount, settings.vsync);
    uint64_t fence_value = 0;
    std::shared_ptr<Fence> fence = device->CreateFence(fence_value);

    Camera camera = Camera(device, app);

    // Load scene
    auto scene = Scene::LoadHardcodedScene(device, camera);

    // Create renderer
    Renderer renderer = Renderer(device, app, camera);

    RenderPassDesc swapchainPassDesc = {
    { { swapchain->GetFormat(), RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore } },
    };
    std::shared_ptr<RenderPass> swapchainRenderPass = device->CreateRenderPass(swapchainPassDesc);

    InputController inputController;
    app.SubscribeEvents((InputEvents*)&inputController, nullptr);

    ScreenShotController screenShotController(glfwGetWin32Window(app.GetWindow()), scene->name);

    inputController.registeredEvents.push_back((InputEvents*)&camera.cameraControls);
    inputController.registeredEvents.push_back((InputEvents*)&renderer.controls);
    inputController.registeredEvents.push_back((InputEvents*)&screenShotController);

    // Create GFX Buffer
    std::array<uint64_t, swapchainTextureCount> fence_values = {};
    std::vector<std::shared_ptr<CommandList>> command_lists;
    std::vector<std::shared_ptr<Framebuffer>> framebuffers;
    for (uint32_t i = 0; i < swapchainTextureCount; ++i)
    {
        // Allocate command list and frame buffer for double buffering.
        ViewDesc back_buffer_view_desc = {};
        back_buffer_view_desc.view_type = ViewType::kRenderTarget;
        back_buffer_view_desc.dimension = ViewDimension::kTexture2D;
        std::shared_ptr<Resource> back_buffer = swapchain->GetBackBuffer(i);
        // TODO: check if we need that?
        std::shared_ptr<View> back_buffer_view = device->CreateView(back_buffer, back_buffer_view_desc);
        FramebufferDesc framebuffer_desc = {};
        framebuffer_desc.render_pass = swapchainRenderPass;
        framebuffer_desc.width = appSize.width();
        framebuffer_desc.height = appSize.height();
        framebuffer_desc.colors = { back_buffer_view };
        std::shared_ptr<Framebuffer> framebuffer =
            framebuffers.emplace_back(device->CreateFramebuffer(framebuffer_desc));
        std::shared_ptr<CommandList> command_list =
            command_lists.emplace_back(device->CreateCommandList(CommandListType::kGraphics));
    }

    while (!app.PollEvents())
    {
        // Wait for the driver to release the lock
        uint32_t frame_index = swapchain->NextImage(fence, ++fence_value);
        command_queue->Wait(fence, fence_value);
        fence->Wait(fence_values[frame_index]);
        
        RenderDoc::StartFrameCapture();

        // Update camera controls and GPU buffer
        camera.UpdateCamera(appSize);

        auto currentSwapchain = swapchain->GetBackBuffer(frame_index);
        renderer.UpdateCommandList(command_lists[frame_index], currentSwapchain, camera, scene);
        
        // Then execute the rendering commands on the GPU.
        command_queue->ExecuteCommandLists({ command_lists[frame_index] });

        command_queue->Signal(fence, fence_values[frame_index] = ++fence_value);
        swapchain->Present(fence, fence_values[frame_index]);

        RenderDoc::EndFrameCapture();
    }
    command_queue->Signal(fence, ++fence_value);
    fence->Wait(fence_value);
    return 0;
}
