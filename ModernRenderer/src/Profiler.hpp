#pragma once

#include "glm/glm.hpp"
#include <memory>
#include <ios>
#include <sstream>
#include <algorithm>
#include <chrono>
#include "ImGuiProfilerRenderer.h"

#include "Instance/Instance.h"
#include <Device/DXDevice.h>

class Profiler
{
private:
	struct Marker
	{
		std::string name;
		int startIndex;
		int endIndex;
		uint64_t startTime;
		uint64_t endTime;
		double elapsedTimeMillis;
	};

	static Profiler instance;

	std::shared_ptr<Device> device;
	ImGuiUtils::ProfilersWindow* profilersWindow;
	std::vector<legit::ProfilerTask> frameGPUTimes;
	ComPtr<ID3D12QueryHeap> queryHeap;
	std::shared_ptr<Resource> readbackBuffer;
	int frameQueryIndex = 0;
	int queryOffset = 0;
	std::vector<Marker> frameMarkers;

	Profiler() = default;
	~Profiler();

public:
	static void Init(std::shared_ptr<Device> device);

	static void DrawImGUIPanel();

	static void BeginFrame();
	static void EndFrame(std::shared_ptr<CommandList> cmd);

	static void BeginMarker(std::shared_ptr<CommandList> cmd, const std::string& name);
	static void EndMarker(std::shared_ptr<CommandList> cmd);

	static void ReadbackStats(std::shared_ptr<CommandQueue> cmdQueue);
};