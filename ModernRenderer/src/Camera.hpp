#pragma once

#include "Instance/Instance.h"
#include "MatrixUtils.hpp"
#include "AppBox/AppBox.h"

struct GPUCameraData
{
    glm::mat4 viewMatrix;
    glm::mat4 inverseViewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 inverseProjectionMatrix;
    glm::mat4 viewProjectionMatrix;
    glm::mat4 inverseViewProjectionMatrix;
    glm::vec4 cameraPosition;
};

class Camera
{
private:
    class CameraControls : InputEvents
    {
    private:
        glm::vec2 lastCursorPos;

    public:
        glm::vec3 movement;
        glm::vec2 rotation;

        CameraControls() : movement(0), rotation(0), lastCursorPos(0) {}

        void OnKey(int key, int action) override;
        void OnMouse(bool first, double xpos, double ypos) override;

        void Reset();
    };

    std::shared_ptr<Device> device;
    CameraControls cameraControls;

public:
    glm::vec3 position;
    glm::vec2 rotation;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;

    GPUCameraData gpuData;
    std::shared_ptr<Resource> cameraDataBuffer;
    std::shared_ptr<View> cameraDataView;
    BindingDesc cameraDataDescFragment;
    BindingDesc cameraDataDescVertex;
    BindingDesc cameraDataDescCompute;
    BindKey cameraDataKeyFragment = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };
    BindKey cameraDataKeyVertex = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };
    BindKey cameraDataKeyCompute = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };

    Camera(std::shared_ptr<Device> device, AppBox& app);

    void UpdateCamera(const AppSize& size);
};