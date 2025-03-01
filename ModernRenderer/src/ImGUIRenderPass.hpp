#pragma once

#include "Device/Device.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <AppBox/AppBox.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_glfw.h>

class ImGUIRenderPass
{
public:
    ImGUIRenderPass(std::shared_ptr<Device> device, AppBox& app);
    ~ImGUIRenderPass();

    void OnUpdate();
    void OnRender(std::shared_ptr<CommandList> cmd);

private:
    std::shared_ptr<Device> device;
    ID3D12DescriptorHeap* cbvSrvHeap;
    int width;
    int height;
    GLFWwindow* window;
};