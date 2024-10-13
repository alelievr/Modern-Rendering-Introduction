#define GLM_FORCE_LEFT_HANDED  // For DirectX compatibility
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  // DirectX depth range is [0,1]
#include <glm/glm.hpp>

#include "Camera.hpp"
#include <glm/gtx/rotate_vector.hpp> 

Camera::Camera(std::shared_ptr<Device> device, AppBox & app)
{
	position = glm::vec3(0, 0, -5);
	rotation = glm::vec2(0, 0);

    cameraDataBuffer = device->CreateBuffer(BindFlag::kConstantBuffer | BindFlag::kCopyDest, sizeof(GPUCameraData));
    cameraDataBuffer->CommitMemory(MemoryType::kUpload);

    ViewDesc constant_view_desc = {};
    constant_view_desc.view_type = ViewType::kConstantBuffer;
    constant_view_desc.dimension = ViewDimension::kBuffer;
    cameraDataView = device->CreateView(cameraDataBuffer, constant_view_desc);

    // Prepare binding keys for all shader types
    cameraDataKeyFragment = { ShaderType::kPixel, ViewType::kConstantBuffer, 0, 0, 1, UINT32_MAX };
    cameraDataKeyVertex = { ShaderType::kVertex, ViewType::kConstantBuffer, 0, 0, 1, UINT32_MAX };
    cameraDataKeyCompute = { ShaderType::kCompute, ViewType::kConstantBuffer, 0, 0, 1, UINT32_MAX };

	cameraDataDescFragment = { cameraDataKeyFragment, cameraDataView };
	cameraDataDescVertex = { cameraDataKeyVertex, cameraDataView };
	cameraDataDescCompute = { cameraDataKeyCompute, cameraDataView };

    app.SubscribeEvents((InputEvents*)&cameraControls, nullptr);

	forward = glm::vec3(0, 0, -1);
}

void Camera::UpdateCamera(const AppSize& size)
{
	rotation += cameraControls.rotation;

	// Build view matrix
	// Note: we don't need to put the camera position in the view matrix, because we're doing camera relative rendering
	glm::mat4x4 view(1);
	view *= MatrixUtils::RotateX(rotation.y) * MatrixUtils::RotateY(rotation.x);

	//view = glm::lookAt(position, position + forward, glm::vec3(0, 1, 0));

	// For now we try that
	//view = MatrixUtils::RotateY(rotation.x);

	auto tranposedView = glm::transpose(view);
	right = glm::vec3(tranposedView[0]);
	up = glm::vec3(tranposedView[1]);
	forward = glm::vec3(tranposedView[2]);

    // Free camera controls
    position += right * cameraControls.movement.x + up * cameraControls.movement.y + forward * cameraControls.movement.z;

	float aspect = size.width() / (float)size.height();
	// glm::mat4x4 projection = MatrixUtils::Perspective(90.0f, aspect, 0.1f, 100.0f);
	//glm::mat4x4 projection = MatrixUtils::Perspective(90.0f, aspect, 0.1f, 100.0f);
	glm::mat4x4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    gpuData.viewMatrix = (view);
	gpuData.inverseViewMatrix = inverse(gpuData.viewMatrix);
	gpuData.projectionMatrix = transpose(projection);
	gpuData.inverseProjectionMatrix = inverse(gpuData.projectionMatrix);
	gpuData.viewProjectionMatrix = transpose(projection * view);
	gpuData.inverseViewProjectionMatrix = inverse(gpuData.viewProjectionMatrix);
	gpuData.cameraPosition = glm::vec4(position, 0);

	cameraDataBuffer->UpdateUploadBuffer(0, &gpuData, sizeof(GPUCameraData));

	cameraControls.Reset();
}

void Camera::CameraControls::OnKey(int key, int action)
{
	float speed = 1.0f / 60.0f;

	if (sprint)
		speed *= 5;

    if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS)
			movement.z = speed;
		else if (action == GLFW_RELEASE)
			movement.z = 0;
	}
	if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			movement.z = -speed;
		else if (action == GLFW_RELEASE)
			movement.z = 0;
	}
	if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
			movement.x = -speed;
		else if (action == GLFW_RELEASE)
			movement.x = 0;
	}
	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
			movement.x = speed;
		else if (action == GLFW_RELEASE)
			movement.x = 0;
	}
	if (key == GLFW_KEY_LEFT_SHIFT)
	{
		if (action == GLFW_PRESS)
			sprint = true;
		else if (action == GLFW_RELEASE)
			sprint = false;
	}
}

void Camera::CameraControls::OnMouse(bool first, double xpos, double ypos)
{
	float rotationSpeed = 2.0f / 60.0f;

	if (first)
	{
		lastCursorPos = glm::vec2(xpos, ypos);
		return;
	}

	glm::vec2 delta = glm::vec2(xpos, ypos) - lastCursorPos;
	rotation = delta * rotationSpeed;

	lastCursorPos = glm::vec2(xpos, ypos);
}

void Camera::CameraControls::Reset()
{
	rotation = glm::vec2(0);
}
