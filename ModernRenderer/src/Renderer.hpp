#pragma once
#include "AppBox/AppBox.h"
#include "Instance/Instance.h"
#include "Camera.hpp"
#include "Scene.hpp"
#include "RenderPipeline.hpp"

class Renderer
{
private:

    enum class RendererMode
    {
        Rasterization = 0,
        PathTracing = 1,
    };

    class Controls : InputEvents
    {
    public:
		RendererMode rendererMode = RendererMode::Rasterization;
        bool screenshotNextFrame = false;

        Controls() {}

        void OnKey(int key, int action) override;
        void OnMouse(bool first, double xpos, double ypos) override {}
    };

    AppSize appSize = { 0, 0 };
    std::shared_ptr<Device> device;
    Camera* camera;
    std::shared_ptr<RenderPipeline> renderPipeline;

    // Render passes
    std::shared_ptr<RenderPass> loadStoreColorRenderPass;
    std::shared_ptr<RenderPass> clearColorRenderPass;

    // Textures
    std::shared_ptr<Resource> mainColorTexture; // Store the color of the scene
    std::shared_ptr<Resource> mainDepthTexture; // Store the depth of the scene
    std::shared_ptr<View> mainColorTextureView;
    std::shared_ptr<View> mainColorRenderTargetView;
    std::shared_ptr<View> mainDepthTextureView;

    // Shader programs
    std::shared_ptr<Program> pathTracingProgram;
    std::shared_ptr<Program> meshShaderProgram;
    std::shared_ptr<Shader> pathTracingLibrary;
    std::shared_ptr<Shader> pathTracingHitLibrary;
    std::shared_ptr<Shader> pathTracingCallableLibrary;

    // Path tracer resources
    std::shared_ptr<Pipeline> pathTracerPipeline;
    std::shared_ptr<BindingSet> pathTracerBindingSet;
    RayTracingShaderTables shaderTables = {};

    // FrameBuffers
    std::shared_ptr<Framebuffer> mainColorFrameBuffer;

    void AllocateRenderTargets();
    void CompileShaders();
    void CreatePipelineObjects();

    void RenderRasterization(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene);
    void RenderPathTracing(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene);

public:
    Controls controls;

	Renderer(std::shared_ptr<Device> device, AppBox& app, Camera& camera);
	~Renderer();
	void UpdateCommandList(std::shared_ptr<CommandList> commandList, std::shared_ptr<Resource> backBuffer, const Camera& camera, std::shared_ptr<Scene> scene);
};
