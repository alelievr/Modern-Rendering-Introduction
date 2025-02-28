#pragma once

#include "Device/Device.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <AppBox/AppBox.h>

class ImGUIRenderPass
{
public:
    ImGUIRenderPass(std::shared_ptr<Device> device, AppBox& app);
    ~ImGUIRenderPass();

    void OnUpdate();
    void OnRender(std::shared_ptr<CommandList> cmd);

private:
    std::shared_ptr<Device> device;
    int width;
    int height;
    GLFWwindow* window;
};