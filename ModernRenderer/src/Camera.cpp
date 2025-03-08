#define GLM_FORCE_LEFT_HANDED  // For DirectX compatibility
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  // DirectX depth range is [0,1]
#include <glm/glm.hpp>

#include "Camera.hpp"
#include <glm/gtx/rotate_vector.hpp> 
#include "RenderSettings.hpp"

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
	cameraDataKeyMesh = { ShaderType::kMesh, ViewType::kConstantBuffer, 0, 0, 1, UINT32_MAX };
	cameraDataKeyAmplification = { ShaderType::kAmplification, ViewType::kConstantBuffer, 0, 0, 1, UINT32_MAX };
    cameraDataKeyVertex = { ShaderType::kVertex, ViewType::kConstantBuffer, 0, 0, 1, UINT32_MAX };
    cameraDataKeyCompute = { ShaderType::kCompute, ViewType::kConstantBuffer, 0, 0, 1, UINT32_MAX };

	cameraDataDescFragment = { cameraDataKeyFragment, cameraDataView };
	cameraDataDescMesh = { cameraDataKeyMesh, cameraDataView };
	cameraDataDescAmplification = { cameraDataKeyAmplification, cameraDataView };
	cameraDataDescVertex = { cameraDataKeyVertex, cameraDataView };
	cameraDataDescCompute = { cameraDataKeyCompute, cameraDataView };

	window = app.GetWindow();
    app.SubscribeEvents((InputEvents*)&cameraControls, nullptr);

	forward = glm::vec3(0, 0, -1);
}

void Camera::UpdateCamera(const AppSize& size)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		cameraControls.Reset();
	}

	rotation += cameraControls.rotation;

	// Build view matrix
	// Note: we don't need to put the camera position in the view matrix, because we're doing camera relative rendering
	glm::mat4x4 view(1);
	view *= MatrixUtils::RotateX(rotation.y) * MatrixUtils::RotateY(rotation.x);

	auto tranposedView = glm::transpose(view);
	right = glm::vec3(tranposedView[0]);
	up = glm::vec3(tranposedView[1]);
	forward = glm::vec3(tranposedView[2]);

    // Free camera controls
    position += right * cameraControls.movement.x + up * cameraControls.movement.y + forward * cameraControls.movement.z;

	float aspect = size.width() / (float)size.height();
	float nearPlane = 0.01f;
	float farPlane = 1000.0f;
	float fov = 45.0f;
	glm::mat4x4 projection = MatrixUtils::Perspective(fov, aspect, nearPlane, farPlane);
	//glm::mat4x4 projection = MatrixUtils::Orthographic(glm::vec2(5), aspect, 0.1f, 1000.0f);

	if (!RenderSettings::freezeFrustumCulling)
	{
		cullingViewProjMatrix = projection * view;
		cullingPosition = position;
	}

    gpuData.viewMatrix = (view);
	gpuData.inverseViewMatrix = inverse(gpuData.viewMatrix);
	gpuData.projectionMatrix = transpose(projection);
	gpuData.inverseProjectionMatrix = inverse(gpuData.projectionMatrix);
	gpuData.viewProjectionMatrix = transpose(projection * view);
	gpuData.inverseViewProjectionMatrix = inverse(gpuData.viewProjectionMatrix);
	gpuData.cameraPosition = glm::vec4(position, 0);
	gpuData.cameraCullingPosition = glm::vec4(cullingPosition, 0);
	gpuData.cameraResolution = glm::vec4(size.width(), size.height(), 1.0f / size.width(), 1.0f / size.height()); // The camera has the same resoution as the window.
	gpuData.orthographicCamera = false;
	gpuData.nearPlane = nearPlane;
	gpuData.farPlane = farPlane;
	gpuData.fieldOfView = glm::radians(fov);
	gpuData.frutsum = MatrixUtils::GetFrustum(projection * view);
	gpuData.cullingFrutsum = MatrixUtils::GetFrustum(cullingViewProjMatrix);
	gpuData.cameraInstanceFrustumCullingDisabled = RenderSettings::frustumInstanceCullingDisabled;
	gpuData.cameraMeshletFrustumCullingDisabled = RenderSettings::frustumMeshletCullingDisabled;
	gpuData.cameraMeshletBackfaceCullingDisabled = RenderSettings::backfacingMeshletCullingDisabled;


	cameraDataBuffer->UpdateUploadBuffer(0, &gpuData, sizeof(GPUCameraData));

	cameraControls.Reset();
}

void Camera::CameraControls::CheckKeyMask(unsigned& mask, CameraKey c, int expectedKey, int key, int action)
{
	if (key == expectedKey)
	{
		if (action == GLFW_PRESS)
			mask |= c;
		else if (action == GLFW_RELEASE)
			mask &= ~c;
	}
}

void Camera::CameraControls::OnKey(int key, int action)
{
	CheckKeyMask(activeKeyMask, Up, GLFW_KEY_E, key, action);
	CheckKeyMask(activeKeyMask, Down, GLFW_KEY_Q, key, action);
	CheckKeyMask(activeKeyMask, Forwad, GLFW_KEY_W, key, action);
	CheckKeyMask(activeKeyMask, Backward, GLFW_KEY_S, key, action);
	CheckKeyMask(activeKeyMask, Left, GLFW_KEY_A, key, action);
	CheckKeyMask(activeKeyMask, Right, GLFW_KEY_D, key, action);
	CheckKeyMask(activeKeyMask, Sprint, GLFW_KEY_LEFT_SHIFT, key, action);

	float speed = currentSpeed;
	if ((activeKeyMask & Sprint) != 0)
		speed *= 5;

	movement = glm::vec3(0);
	if (activeKeyMask & Forwad)
		movement.z += speed;
	if (activeKeyMask & Backward)
		movement.z -= speed;
	if (activeKeyMask & Right)
		movement.x += speed;
	if (activeKeyMask & Left)
		movement.x -= speed;
	if (activeKeyMask & Up)
		movement.y += speed;
	if (activeKeyMask & Down)
		movement.y -= speed;
}

void Camera::CameraControls::OnScroll(double xoffset, double yoffset)
{
	currentSpeed += yoffset * 0.04f;
	if (currentSpeed < 0.01f)
		currentSpeed = 0.01f;
}

void Camera::CameraControls::OnMouse(bool first, double xpos, double ypos)
{
	float rotationSpeed = 0.5f / 60.0f;

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
