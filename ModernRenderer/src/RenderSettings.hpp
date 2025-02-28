#pragma once

class RenderSettings
{
public:
	RenderSettings() = delete;
	~RenderSettings() = delete;

	static bool frustumCulling;
	static bool freezeFrustumCulling;

	static void RenderImGUISettingsWindow();
};
