#pragma once

class RenderSettings
{
public:
	RenderSettings() = delete;
	~RenderSettings() = delete;

	// Rasterization settings
	static bool frustumInstanceCullingDisabled;
	static bool frustumMeshletCullingDisabled;
	static bool backfacingMeshletCullingDisabled;
	static bool freezeFrustumCulling;
	static bool noUI;

	// Path tracing settings
	static int integrationCountPerFrame;
	static int MaxAccumulationCount;

	static void RenderImGUISettingsWindow();
};
