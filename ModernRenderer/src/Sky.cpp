#include "Sky.hpp"
#include <filesystem>
#include "RenderUtils.hpp"

void ConvertFloatArrayToHalfInPlace(float* image, int width, int height)
{
    int numPixels = width * height * 4;
    uint16_t* halfImage = (uint16_t*)image;

    for (int i = 0; i < numPixels; ++i) {
        halfImage[i] = glm::detail::toFloat16(image[i]);
    }
}

void Sky::LoadHDRI(std::shared_ptr<Device> device, const char* filepath)
{
    int width, height, channels;
    void* image = stbi_loadf(filepath, &width, &height, NULL, STBI_rgb_alpha);
    channels = 4;

    // STBI can only load 32 bit float images, convert to 16 bit float
    ConvertFloatArrayToHalfInPlace((float*)image, width, height);

    if (image == nullptr)
    {
        printf("Failed to load HDRI Image: %s\n", filepath);
        return;
    }

    gli::format format = gli::FORMAT_RGBA16_SFLOAT_PACK16;

    // compute mip levels
    int mipCount = std::floor(std::log2(std::max(width, height))) + 1;
    hdriSkyTexture = device->CreateTexture(
        TextureType::k2D,
        BindFlag::kShaderResource | BindFlag::kCopyDest,
        format,
        1, width, height, 1, mipCount
    );
    hdriSkyTexture->CommitMemory(MemoryType::kDefault);

    // Set the file name to the resource for debugging
    std::filesystem::path p(filepath);
    hdriSkyTexture->SetName(p.filename().string());

    ViewDesc viewDesc = {};
    viewDesc.dimension = ViewDimension::kTexture2D;
    viewDesc.view_type = ViewType::kTexture;
    hdriSkyTextureView = device->CreateView(hdriSkyTexture, viewDesc);

    RenderUtils::UploadTextureData(hdriSkyTexture, device, 0, image, width, height, channels, 2);

    stbi_image_free(image);

    // Load HDRI Sky shader
    std::shared_ptr<Shader> pixelMeshshader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/Sky.hlsl", "mesh", ShaderType::kMesh, "6_5" });
    std::shared_ptr<Shader> meshShader = device->CompileShader(
        { MODERN_RENDERER_ASSETS_PATH "shaders/Sky.hlsl", "fragment", ShaderType::kPixel, "6_5" });
    skyProgram = device->CreateProgram({ meshShader, pixelMeshshader });

    // Create Render pass
    RenderPassDepthStencilDesc depthStencilDesc = {
        gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64,
        RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore, // depth load/store
        RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore // stencil load/store
    };
    RenderPassColorDesc mainColorDesc = { gli::FORMAT_RGBA8_UNORM_PACK8, RenderPassLoadOp::kLoad, RenderPassStoreOp::kStore };
    RenderPassDesc renderPassDesc = {
        { mainColorDesc },
        depthStencilDesc
    };
    skyRenderPass = device->CreateRenderPass(renderPassDesc);

}

void Sky::Initialize(std::shared_ptr<Device> device, Camera* camera)
{
	this->device = device;

    BindKey bindKey = { ShaderType::kPixel, ViewType::kTexture, 0, 4};
    BindingDesc bindingDesc = { bindKey, hdriSkyTextureView };

    skyLayout = RenderUtils::CreateLayoutSet(device, *camera, { bindKey }, RenderUtils::CameraData | RenderUtils::TextureList, RenderUtils::Mesh | RenderUtils::Fragment);
    skyBindingSet = RenderUtils::CreateBindingSet(device, skyLayout, *camera, { bindingDesc }, RenderUtils::CameraData | RenderUtils::TextureList, RenderUtils::Mesh | RenderUtils::Fragment);

    GraphicsPipelineDesc skyPipelineDesc = {
        skyProgram,
        skyLayout,
        {},
        skyRenderPass,
    };
    skyPipelineDesc.rasterizer_desc = { FillMode::kSolid, CullMode::kBack, 0 };
    skyPipelineDesc.depth_stencil_desc = { true, ComparisonFunc::kLessEqual, false };

    skyPipeline = device->CreateGraphicsPipeline(skyPipelineDesc);
}

void Sky::Render(std::shared_ptr<CommandList> cmd, std::shared_ptr<Resource> colorTexture, std::shared_ptr<View> colorTextureView,
    std::shared_ptr<Resource> depthTexture, std::shared_ptr<View> depthTextureView)
{
    if (!skyFramebuffer)
    {
        FramebufferDesc desc = {};
        desc.render_pass = skyRenderPass;
        desc.width = colorTexture->GetWidth();
        desc.height = colorTexture->GetHeight();
        desc.colors = { colorTextureView };
        desc.depth_stencil = depthTextureView;
        skyFramebuffer = device->CreateFramebuffer(desc);
    }

    ClearDesc clearDesc = { { { 0.0, 0.2, 0.4, 1.0 } } }; // Clear Color
    cmd->BindPipeline(skyPipeline);
    cmd->BindBindingSet(skyBindingSet);
    cmd->ResourceBarrier({ { colorTexture, ResourceState::kCommon, ResourceState::kRenderTarget } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kCommon, ResourceState::kDepthStencilWrite } });
    cmd->BeginRenderPass(skyRenderPass, skyFramebuffer, clearDesc);

    // Fullscreen dispatch mesh
    cmd->DispatchMesh(1);

    cmd->EndRenderPass();
    cmd->ResourceBarrier({ { colorTexture, ResourceState::kRenderTarget, ResourceState::kCommon } });
    cmd->ResourceBarrier({ { depthTexture, ResourceState::kDepthStencilWrite, ResourceState::kCommon } });
    // TODO: render pass set, etc.
}
