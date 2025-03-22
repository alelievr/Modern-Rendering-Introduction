#pragma once

#include <stb_image.h>
#include "Instance/Instance.h"
#include "Camera.hpp"

class Sky
{
public:
	enum Type
	{
		HDRI,
		//Procedural, // TODO
	};

	std::shared_ptr<Device> device;

	std::shared_ptr<Resource> hdriSkyTexture;
	std::shared_ptr<View> hdriSkyTextureView;
	std::shared_ptr<Pipeline> skyPipeline;
	std::shared_ptr<RenderPass> skyRenderPass;
	std::shared_ptr<Program> skyProgram;
	std::shared_ptr<BindingSetLayout> skyLayout;
	std::shared_ptr<BindingSet> skyBindingSet;
	std::shared_ptr<Framebuffer> skyFramebuffer;

	static BindKey bindKey;
	static BindingDesc bindingDesc;

	Sky() = default;
	~Sky() = default;

	void LoadHDRI(std::shared_ptr<Device> device, const char* filepath);
	void Initialize(std::shared_ptr<Device> device, Camera* camera);

	void Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> colorTexture, std::shared_ptr<View> colorTextureView, std::shared_ptr<Resource> depthTexture, std::shared_ptr<View> depthTextureView);
};