#pragma once

#include "Instance/Instance.h"
#include "Camera.hpp"
#include "Scene.hpp"

class RenderPipeline
{
private:
	std::shared_ptr<Scene> scene;
	Camera* camera;
	std::shared_ptr<Device> device;
	AppSize appSize = { 0, 0 };

	// Common Render Pipeline resources
	std::shared_ptr<Resource> colorTexture;
	std::shared_ptr<View> colorTextureView;
	std::shared_ptr<Resource> depthTexture;
	std::shared_ptr<View> depthTextureView;

	// Visibility Render Pass resources
	std::shared_ptr<Resource> visibilityTexture;
	std::shared_ptr<View> visibilityRenderTargetView;
	std::shared_ptr<View> visibilityTextureView;
	std::shared_ptr<RenderPass> visibilityRenderPass;
	std::shared_ptr<Framebuffer> visibilityFrameBuffer;
	std::shared_ptr<Pipeline> visibilityPipeline;
	std::shared_ptr<Program> visibilityProgram;
	std::shared_ptr<Shader> visibilityMeshShader;
	std::shared_ptr<Shader> visibilityFragmentShader;

	// Forward Render Pass resources
	std::shared_ptr<RenderPass> forwardRenderPass;
	std::shared_ptr<Framebuffer> forwardFrameBuffer;
	std::shared_ptr<Pipeline> forwardPipeline;
	std::shared_ptr<Program> forwardProgram;
	std::shared_ptr<BindingSetLayout> forwardLayoutSet;
	std::shared_ptr<BindingSet> forwardBindingSet;

	std::shared_ptr<BindingSetLayout> objectLayoutSet;
	std::shared_ptr<BindingSet> objectBindingSet;

	void DrawOpaqueObjects(std::shared_ptr<CommandList> cmd, std::shared_ptr<BindingSet> set, std::shared_ptr<Pipeline> pipeline);
	void MeshletFrustumCulling(std::shared_ptr<CommandList> cmd);
	void RenderVisibility(std::shared_ptr<CommandList> cmd);
	void RenderForwardOpaque(std::shared_ptr<CommandList> cmd);

public:
	RenderPipeline(std::shared_ptr<Device> device, const AppSize& appSize, Camera& camera, std::shared_ptr<Resource> colorTexture, std::shared_ptr<View> colorTextureView, std::shared_ptr<Resource> depthTexture, std::shared_ptr<View> depthTextureView);
	~RenderPipeline() = default;

	void Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, std::shared_ptr<Scene> scene);
};