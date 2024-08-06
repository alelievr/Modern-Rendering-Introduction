#pragma once

#define LITEFX_AUTO_IMPORT_BACKEND_HEADERS
#define LITEFX_BUILD_DIRECTX_12_BACKEND
#include <litefx/litefx.h>

#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("Compute Sample: No supported surface platform detected.")
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <memory>

#include "config.h"

#ifdef LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER
#include <renderdoc_app.h>

extern RENDERDOC_API_1_5_0* renderDoc;
#endif
#include "ShaderCache.h"

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

struct GlfwWindowDeleter {
	void operator()(GLFWwindow* ptr) noexcept {
		::glfwDestroyWindow(ptr);
	}
};

typedef UniquePtr<GLFWwindow, GlfwWindowDeleter> GlfwWindowPtr;

class ModernRenderer : public LiteFX::App {
public:
	static String Name() noexcept { return "LiteFX Sample: Compute"; }
	String name() const noexcept override { return Name(); }

	static AppVersion Version() noexcept { return AppVersion(1, 0, 0, 0); }
	AppVersion version() const noexcept override { return Version(); }

private:
	/// <summary>
	/// Stores the GLFW window pointer.
	/// </summary>
	GlfwWindowPtr m_window;

	/// <summary>
	/// Stores the preferred adapter ID (<c>std::nullopt</c>, if the default adapter is used).
	/// </summary>
	Optional<UInt32> m_adapterId;

	/// <summary>
	/// Stores a reference of the input assembler state.
	/// </summary>
	SharedPtr<IInputAssembler> m_inputAssembler;

	/// <summary>
	/// Stores the viewport.
	/// </summary>
	SharedPtr<IViewport> m_viewport;

	/// <summary>
	/// Stores the scissor.
	/// </summary>
	SharedPtr<IScissor> m_scissor;

	/// <summary>
	/// Stores a pointer to the currently active device.
	/// </summary>
	IGraphicsDevice* m_device;

	/// <summary>
	/// Stores the fence created at application load time.
	/// </summary>
	UInt64 m_transferFence = 0;

	ShaderCache m_ShaderCache;

public:
	ModernRenderer(GlfwWindowPtr&& window, Optional<UInt32> adapterId) :
		App(), m_window(std::move(window)), m_adapterId(adapterId), m_device(nullptr)
	{
		this->initializing += std::bind(&ModernRenderer::onInit, this);
		this->startup += std::bind(&ModernRenderer::onStartup, this);
		this->resized += std::bind(&ModernRenderer::onResize, this, std::placeholders::_1, std::placeholders::_2);
		this->shutdown += std::bind(&ModernRenderer::onShutdown, this);
	}

private:
	/// <summary>
	/// Initializes the buffers.
	/// </summary>
	/// <param name="backend">The render backend to use.</param>
	void initBuffers(IRenderBackend* backend);

	/// <summary>
	/// Updates the camera buffer. This needs to be done whenever the frame buffer changes, since we need to pass changes in the aspect ratio to the view/projection matrix.
	/// </summary>
	void updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer) const;

	template<typename TRenderBackend> requires
		meta::implements<TRenderBackend, IRenderBackend>
		void InitRenderGraph(TRenderBackend* backend, SharedPtr<IInputAssembler>& inputAssemblerState);

private:
	void onInit();
	void onStartup();
	void onShutdown();
	void onResize(const void* sender, ResizeEventArgs e);

public:
	void keyDown(int key, int scancode, int action, int mods);
	void handleEvents();
	void drawFrame();
	void updateWindowTitle();
};