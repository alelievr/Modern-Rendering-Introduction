#pragma once

class RenderSettings
{
public:
	RenderSettings() = delete;
	~RenderSettings() = delete;

	static bool frustumInstanceCullingDisabled;
	static bool frustumMeshletCullingDisabled;
	static bool freezeFrustumCulling;
	static bool noUI;

	static void RenderImGUISettingsWindow();
};
