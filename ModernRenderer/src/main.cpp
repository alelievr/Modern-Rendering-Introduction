#include "AppBox/AppBox.h"
#include "AppSettings/ArgsParser.h"
#include "Instance/Instance.h"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "InputController.hpp"
#include "RenderDoc.hpp"
#include "ScreenShotController.hpp"
#include "RenderUtils.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "RenderSettings.hpp"

//#define LOAD_RENDERDOC
//#define FORCE_BACKGROUND_BLACK

int main(int argc, char* argv[])
{
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    Settings settings = ParseArgs(argc, argv);
    AppBox app("ModernRenderer", settings);
    AppSize appSize = app.GetAppSize();

#if defined(FORCE_BACKGROUND_BLACK)
    // Set window color back while waiting for the rest to load
    RenderUtils::SetBackgroundColor(app.GetWindow(), RGB(0, 0, 0));
#endif

#if defined(LOAD_RENDERDOC)
    // Loading renderdoc will diisable the validation layer, make sure there is no error before enabling it.
    RenderDoc::LoadRenderDoc();
    RenderSettings::noUI = true;
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

    InputController inputController;
    app.SubscribeEvents((InputEvents*)&inputController, nullptr);

    ScreenShotController screenShotController(glfwGetWin32Window(app.GetWindow()), scene->name);

    inputController.registeredEvents.push_back((InputEvents*)&camera.cameraControls);
    inputController.registeredEvents.push_back((InputEvents*)&renderer.controls);
    inputController.registeredEvents.push_back((InputEvents*)&screenShotController);

    // Allocate command lists for each swapchain image
    std::array<uint64_t, swapchainTextureCount> fence_values = {};
    std::vector<std::shared_ptr<CommandList>> command_lists;
    for (uint32_t i = 0; i < swapchainTextureCount; ++i)
    {
        auto cmd = device->CreateCommandList(CommandListType::kGraphics);
        cmd->SetName("Main Rendering");
        command_lists.emplace_back(cmd);
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
