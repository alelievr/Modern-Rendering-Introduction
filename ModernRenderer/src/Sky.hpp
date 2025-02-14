#pragma once

#include <stb_image.h>
#include "Instance/Instance.h"

class Sky
{
public:
	enum Type
	{
		HDRI,
		//Procedural, // TODO
	};

	Sky();
	~Sky();

	std::shared_ptr<Resource> hdriSkyTexture;
	std::shared_ptr<View> hdriSkyTextureView;
	std::shared_ptr<Pipeline> skyPipeline;
	std::shared_ptr<RenderPass> skyRenderPass;
	std::shared_ptr<Program> skyProgram;
	std::shared_ptr<BindingSetLayout> skyLayout;
	std::shared_ptr<BindingSet> skyBindingSet;

	void LoadHDRI(std::shared_ptr<Device> device, const char* filepath);
	void Initialize(std::shared_ptr<Device> device, Camera* camera);

	void Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> colorBuffer, std::shared_ptr<Resource> depthBuffer);
};