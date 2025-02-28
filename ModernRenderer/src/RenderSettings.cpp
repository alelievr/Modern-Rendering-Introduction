#include "RenderSettings.hpp"
#include <imgui.h>

bool RenderSettings::frustumCulling = true;
bool RenderSettings::freezeFrustumCulling = false;

void RenderSettings::RenderImGUISettingsWindow()
{
    ImGui::SetNextWindowDockID(ImGui::GetID("DockLeft"), ImGuiCond_FirstUseEver);

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Checkbox("Frustum culling", &frustumCulling);
    ImGui::Checkbox("Freeze frustum culling", &freezeFrustumCulling);

    ImGui::End();
}
