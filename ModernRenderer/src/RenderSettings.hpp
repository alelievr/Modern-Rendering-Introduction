#pragma once

class RenderSettings
{
public:
	RenderSettings() = delete;
	~RenderSettings() = delete;

	static bool frustumCullingDisabled;
	static bool freezeFrustumCulling;
	static bool noUI;

	static void RenderImGUISettingsWindow();
};
