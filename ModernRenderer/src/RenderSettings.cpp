#include "RenderSettings.hpp"
#include <imgui.h>

bool RenderSettings::frustumInstanceCullingDisabled = false;
bool RenderSettings::frustumMeshletCullingDisabled = false;
bool RenderSettings::backfacingMeshletCullingDisabled = false;
bool RenderSettings::freezeFrustumCulling = false;
bool RenderSettings::noUI = false;

int RenderSettings::integrationCountPerFrame = 1;
int RenderSettings::MaxAccumulationCount = 1024;

void RenderSettings::RenderImGUISettingsWindow()
{
    ImGui::SetNextWindowDockID(ImGui::GetID("DockLeft"), ImGuiCond_FirstUseEver);

    ImGui::Begin("Rasterization Settings", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Checkbox("Disable Instance Frustum culling", &frustumInstanceCullingDisabled);
    ImGui::Checkbox("Disable Meshlet Frustum culling", &frustumMeshletCullingDisabled);
    ImGui::Checkbox("Disable Backfacing Meshlet culling", &backfacingMeshletCullingDisabled);
    ImGui::Checkbox("Freeze frustum culling", &freezeFrustumCulling);

    ImGui::End();

    ImGui::Begin("Path Tracing Settings", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::SliderInt("Integration count per frame", &integrationCountPerFrame, 1, 100);
    ImGui::InputInt("Max Accumulation Count", &MaxAccumulationCount);

    ImGui::End();
}
