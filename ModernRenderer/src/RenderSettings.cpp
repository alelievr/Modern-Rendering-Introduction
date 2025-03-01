#include "RenderSettings.hpp"
#include <imgui.h>

bool RenderSettings::frustumCullingDisabled = false;
bool RenderSettings::freezeFrustumCulling = false;
bool RenderSettings::noUI = false;

void RenderSettings::RenderImGUISettingsWindow()
{
    ImGui::SetNextWindowDockID(ImGui::GetID("DockLeft"), ImGuiCond_FirstUseEver);

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Checkbox("Disable Frustum culling", &frustumCullingDisabled);
    ImGui::Checkbox("Freeze frustum culling", &freezeFrustumCulling);

    ImGui::End();
}
