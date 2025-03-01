#include "ImGUIRenderPass.hpp"

#include "Device/DXDevice.h"
#include "Resource/DXResource.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#include <CommandList/DXCommandList.h>
#include "RenderSettings.hpp"
#include <View/DXView.h>

ImGUIRenderPass::ImGUIRenderPass(std::shared_ptr<Device> device, AppBox& app)
    : device(device)
    , width(app.GetAppSize().width())
    , height(app.GetAppSize().height())
    , window(app.GetWindow())
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2((float)width, (float)height);
#if defined(_WIN32)
    UINT dpi = GetDpiForWindow(glfwGetWin32Window(window));
    io.FontGlobalScale = dpi / 96.0f;
#else
    io.FontGlobalScale = 2.0f;
#endif
    ImGui::GetStyle().ScaleAllSizes(io.FontGlobalScale);

    ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 1;  // Ensure at least one descriptor
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    DXDevice* dxDevice = (DXDevice*)device.get();
    dxDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&cbvSrvHeap));

    D3D12_CPU_DESCRIPTOR_HANDLE fontSrvCpuDescHandle = cbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE fontSrvGpuDescHandle = cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();

    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplDX12_Init(dxDevice->GetDevice().Get(), 2, DXGI_FORMAT_R8G8B8A8_UNORM, cbvSrvHeap, fontSrvCpuDescHandle, fontSrvGpuDescHandle);
}

ImGUIRenderPass::~ImGUIRenderPass()
{
    ImGui_ImplDX12_Shutdown();
	ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGUIRenderPass::OnUpdate() {}

void ImGUIRenderPass::OnRender(std::shared_ptr<CommandList> cmd)
{
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    RenderSettings::RenderImGUISettingsWindow();

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    DXCommandList* dxCmd = (DXCommandList*)cmd.get();
    auto nativeCmd = dxCmd->GetCommandList().Get();
    ID3D12DescriptorHeap* descriptorHeaps[] = { cbvSrvHeap };
    nativeCmd->SetDescriptorHeaps(1, descriptorHeaps);
    ImGui_ImplDX12_RenderDrawData(draw_data, nativeCmd);

    ImGui::EndFrame();
}
