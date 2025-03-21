#include "Profiler.hpp"
#include <CommandList/DXCommandList.h>
#include <Resource/DXResource.h>
#include <CommandQueue/DXCommandQueue.h>

Profiler Profiler::instance;

void Profiler::Init(std::shared_ptr<Device> device)
{
	instance.device = device;
	auto dxDevice = (DXDevice*)device.get();
	auto nativeDevice = dxDevice->GetDevice();

	D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	queryHeapDesc.Count = 1024;
	nativeDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&instance.queryHeap));

	instance.readbackBuffer = device->CreateBuffer(BindFlag::kCopyDest, 1024 * sizeof(uint64_t));
	instance.readbackBuffer->CommitMemory(MemoryType::kReadback);
	instance.readbackBuffer->SetName("Timings Readback Buffer");

	instance.profilersWindow = new ImGuiUtils::ProfilersWindow();
	instance.profilersWindow->frameWidth = 1;
	instance.profilersWindow->frameSpacing = 0;
}

Profiler::~Profiler()
{
	queryHeap->Release();
}

void Profiler::DrawImGUIPanel()
{
	instance.profilersWindow->Render();
}

void Profiler::BeginFrame()
{
	instance.frameQueryIndex = 0;
	instance.frameMarkers.clear();
	instance.frameGPUTimes.clear();
	instance.frameCPUTimes.clear();
}

void Profiler::EndFrame(std::shared_ptr<CommandList> cmd)
{
	auto buf = instance.readbackBuffer->As<DXResource>().resource.Get();
	cmd->As<DXCommandList>().GetCommandList()->ResolveQueryData(instance.queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, instance.frameQueryIndex, buf, 0);
}

double getCurrentTimeInSeconds()
{
	LARGE_INTEGER frequency, counter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return static_cast<double>(counter.QuadPart) / frequency.QuadPart;
}

void Profiler::BeginMarker(std::shared_ptr<CommandList> cmd, const std::string& name)
{
	cmd->BeginEvent(name);

	cmd->As<DXCommandList>().GetCommandList()->EndQuery(instance.queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, instance.frameQueryIndex);
	Marker marker;
	marker.name = name;
	marker.startIndex = instance.frameQueryIndex;
	marker.startCPUTime = getCurrentTimeInSeconds();

	instance.markerIndexStack.push(instance.frameMarkers.size());
	instance.frameMarkers.push_back(marker);
	instance.frameQueryIndex++;
}

void Profiler::EndMarker(std::shared_ptr<CommandList> cmd)
{
	cmd->EndEvent();
	if (instance.frameMarkers.empty()) return;
	unsigned index = instance.markerIndexStack.top();
	instance.markerIndexStack.pop();
	auto& marker = instance.frameMarkers[index];
	cmd->As<DXCommandList>().GetCommandList()->EndQuery(instance.queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, instance.frameQueryIndex);
	marker.endIndex = instance.frameQueryIndex;
	marker.endCPUTime = getCurrentTimeInSeconds();

	instance.frameQueryIndex++;
}

uint32_t GetColorFromString(const std::string& input)
{
	std::hash<std::string> hasher;
	float hue = (hasher(input) * 2654435761u % 360) / 360.0f;

	float s = 0.7f, v = 0.95f;

	hue -= std::floor(hue);
	float r = std::fmax(0.0f, std::abs(hue * 6 - 3) - 1);
	float g = std::fmax(0.0f, 2 - std::abs(hue * 6 - 2));
	float b = std::fmax(0.0f, 2 - std::abs(hue * 6 - 4));

	// Apply S and V
	r = v * (1 - s + s * r);
	g = v * (1 - s + s * g);
	b = v * (1 - s + s * b);

	return  ((int(r * 255) & 0xFF) << 0) |
			((int(g * 255) & 0xFF) << 8) |
			((int(b * 255) & 0xFF) << 16) |
			255 << 24;
}

void Profiler::ReadbackStats(std::shared_ptr<CommandQueue> cmdQueue)
{
	if (instance.frameMarkers.size() == 0)
		return;

	uint64_t* timings = (uint64_t*)instance.readbackBuffer->Map();
	uint64_t frequency;
	DXCommandQueue* dxQueue = (DXCommandQueue*)cmdQueue.get();
	auto nativeQueue = dxQueue->GetQueue();
	nativeQueue->GetTimestampFrequency(&frequency);

	uint64_t firstMarkerGPUFrameSec = timings[0];
	auto firstMarkerCPUTime = instance.frameMarkers[0].startCPUTime;

	for (auto& marker : instance.frameMarkers)
	{
		marker.startGPUTime = timings[marker.startIndex];
		marker.endGPUTime = timings[marker.endIndex];
		marker.elapsedTimeMillis = (double)(marker.endGPUTime - marker.startGPUTime) / frequency * 1000.0;
		double startGPUTimeSec = (double)(marker.startGPUTime - firstMarkerGPUFrameSec) / frequency;
		double endGPUTimeSec = (double)(marker.endGPUTime - firstMarkerGPUFrameSec) / frequency;
		legit::ProfilerTask taskGPU = { startGPUTimeSec, endGPUTimeSec, marker.name, GetColorFromString(marker.name) };
		instance.frameGPUTimes.push_back(taskGPU);

		double startCPUTime = marker.startCPUTime - firstMarkerCPUTime;
		double endCPUTime = marker.endCPUTime - firstMarkerCPUTime;

		legit::ProfilerTask taskCPU = { startCPUTime, endCPUTime, marker.name, GetColorFromString(marker.name) };
		instance.frameCPUTimes.push_back(taskCPU);
	}
	instance.readbackBuffer->Unmap();

	instance.profilersWindow->gpuGraph.LoadFrameData(instance.frameGPUTimes.data(), instance.frameGPUTimes.size());
	instance.profilersWindow->cpuGraph.LoadFrameData(instance.frameCPUTimes.data(), instance.frameCPUTimes.size());
}
