#include "RenderSettings.hpp"
#include <imgui.h>

bool RenderSettings::frustumInstanceCullingDisabled = false;
bool RenderSettings::frustumMeshletCullingDisabled = false;
bool RenderSettings::freezeFrustumCulling = false;
bool RenderSettings::noUI = false;

void RenderSettings::RenderImGUISettingsWindow()
{
    ImGui::SetNextWindowDockID(ImGui::GetID("DockLeft"), ImGuiCond_FirstUseEver);

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Checkbox("Disable Instance Frustum culling", &frustumInstanceCullingDisabled);
    ImGui::Checkbox("Disable Meshlet Frustum culling", &frustumMeshletCullingDisabled);
    ImGui::Checkbox("Freeze frustum culling", &freezeFrustumCulling);

    ImGui::End();
}
