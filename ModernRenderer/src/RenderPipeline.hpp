#pragma once

#include "Instance/Instance.h"
#include "Camera.hpp"
#include "Scene.hpp"
#include "RenderUtils.hpp"

#include <CommandList/DXCommandList.h>
#include <Resource/DXResource.h>

// Set the maximum number of visible meshlets for the different culling steps divided by 65k
// 256 * 65k = 16.7M. This is a very high limit as most of them will get culled.
#define MAX_VISIBLE_MESHLETS 256

class RenderPipeline
{
private:
	std::shared_ptr<Scene> scene;
	Camera* camera;
	std::shared_ptr<Device> device;
	AppSize appSize = { 0, 0 };

	// Frustum culling resources
	std::shared_ptr<Resource> meshletCullingIndirectArgsBuffer;
	std::shared_ptr<View> meshletCullingIndirectArgsView;
	std::shared_ptr<Resource> meshletCullingIndirectCountBuffer;
	std::shared_ptr<View> meshletCullingIndirectCountView;
	std::shared_ptr<Resource> visibleMeshletsCountBuffer;
	std::shared_ptr<View> visibleMeshletsCountView;
	std::shared_ptr<BindingSetLayout> instanceFrustumCullingLayoutSet;
	std::shared_ptr<BindingSet> instanceFrustumCullingSet;
	ComPtr<ID3D12CommandSignature> frustumCullingCommandSignature;
	RenderUtils::ComputeProgram frustumCullingProgram;
	RenderUtils::ComputeProgram frustumCullingClearProgram;
	RenderUtils::ComputeProgram frustumCullingIndirectArgsProgram;
	//std::shared_ptr<Pipeline> frustumCullingPipeline;
	//std::shared_ptr<Pipeline> frustumCullingClearPipeline;
	//std::shared_ptr<Pipeline> frustumCullingIndirectArgsPipeline;
	//std::shared_ptr<Shader> frustumCullingShader;
	//std::shared_ptr<Shader> frustumCullingClearShader;
	//std::shared_ptr<Shader> frustumCullingIndirectArgsShader;
	//std::shared_ptr<Program> frustumCullingProgram;
	//std::shared_ptr<Program> frustumCullingClearProgram;
	//std::shared_ptr<Program> frustumCullingIndirectArgsProgram;

	// Meshlet culling resources
	//std::shared_ptr<Resource> meshletIndirectArgsBuffer;
	//std::shared_ptr<View> meshletIndirectArgsView;
	//std::shared_ptr<Resource> meshletIndirectCountBuffer;
	//std::shared_ptr<View> meshletIndirectCountView;
	RenderUtils::ComputeProgram meshletCullingProgram;
	RenderUtils::ComputeProgram meshletCullingClearProgram;
	RenderUtils::ComputeProgram meshletCullingIndirectArgsProgram;
	ComPtr<ID3D12CommandSignature> meshletCullingCommandSignature;

	//std::shared_ptr<BindingSetLayout> meshletCullingLayoutSet;
	//std::shared_ptr<BindingSet> meshletCullingSet;
	//std::shared_ptr<Pipeline> meshletCullingPipeline;
	//std::shared_ptr<Shader> meshletCullingShader;
	//std::shared_ptr<Program> meshletCullingProgram;

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

	void FrustumCulling(std::shared_ptr<CommandList> cmd);
	void MeshletCulling(std::shared_ptr<CommandList> cmd);
	void RenderVisibility(std::shared_ptr<CommandList> cmd);
	void RenderForwardOpaque(std::shared_ptr<CommandList> cmd);

public:
	RenderPipeline(std::shared_ptr<Device> device, const AppSize& appSize, Camera& camera, std::shared_ptr<Resource> colorTexture, std::shared_ptr<View> colorTextureView, std::shared_ptr<Resource> depthTexture, std::shared_ptr<View> depthTextureView);
	~RenderPipeline() = default;

	void Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> backBuffer, std::shared_ptr<Scene> scene);
};