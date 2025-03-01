#pragma once

#include "Instance/Instance.h"
#include "Camera.hpp"
#include "Scene.hpp"
#include <directx/d3dx12.h>
#include <CommandList/DXCommandList.h>
#include <Resource/DXResource.h>
#include <Device/DXDevice.h>
#include <BindingSetLayout/DXBindingSetLayout.h>

class RenderPipeline
{
private:
	std::shared_ptr<Scene> scene;
	Camera* camera;
	std::shared_ptr<Device> device;
	AppSize appSize = { 0, 0 };

	// Frustum culling resources
	std::shared_ptr<Resource> meshletIndirectArgsBuffer;
	std::shared_ptr<View> meshletIndirectArgsBufferView;
	std::shared_ptr<Resource> meshletIndirectCountBuffer;
	std::shared_ptr<View> meshletIndirectCountBufferView;
	std::shared_ptr<BindingSetLayout> instanceFrustumCullingLayoutSet;
	std::shared_ptr<BindingSet> instanceFrustumCullingSet;
	ComPtr<ID3D12CommandSignature> frustumCullingCommandSignature;
	std::shared_ptr<Pipeline> frustumCullingPipeline;
	std::shared_ptr<Pipeline> frustumCullingClearPipeline;
	std::shared_ptr<Shader> frustumCullingShader;
	std::shared_ptr<Shader> frustumCullingClearShader;
	std::shared_ptr<Program> frustumCullingProgram;
	std::shared_ptr<Program> frustumCullingClearProgram;

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
	std::shared_ptr<Shader> visibilityTaskShader;
	std::shared_ptr<Shader> visibilityFragmentShader;
	std::shared_ptr<BindingSetLayout> indirectVisibilityLayoutSet;
	std::shared_ptr<BindingSet> indirectVisibilitySet;

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
	void FrustumCulling(std::shared_ptr<CommandList> cmd);
	void RenderVisibility(std::shared_ptr<CommandList> cmd);
	void RenderForwardOpaque(std::shared_ptr<CommandList> cmd);

public:
	RenderPipeline(std::shared_ptr<Device> device, const AppSize& appSize, Camera& camera, std::shared_ptr<Resource> colorTexture, std::shared_ptr<View> colorTextureView, std::shared_ptr<Resource> depthTexture, std::shared_ptr<View> depthTextureView);
	~RenderPipeline() = default;

	void Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, std::shared_ptr<Scene> scene);
};