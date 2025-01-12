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
    glm::vec4 cameraResolution;
};

class Camera
{
private:
    enum CameraKey
    {
        Forwad = 1 << 0,
        Backward = 1 << 1,
        Right = 1 << 2,
        Left = 1 << 3,
        Up = 1 << 4,
        Down = 1 << 5,
        Sprint = 1 << 6
    };

    class CameraControls : InputEvents
    {
    private:
        glm::vec2 lastCursorPos;
        unsigned activeKeyMask;

        void CheckKeyMask(unsigned& mask, CameraKey c, int expectedKey, int key, int action);

    public:
        glm::vec3 movement;
        glm::vec2 rotation;

        CameraControls() : movement(0), rotation(0), lastCursorPos(0), activeKeyMask(0) {}

        void OnKey(int key, int action) override;
        void OnMouse(bool first, double xpos, double ypos) override;

        void Reset();
    };

    std::shared_ptr<Device> device;

    // Disable copy constructor
    Camera(const Camera&);
    Camera& operator=(const Camera&);

public:
    CameraControls cameraControls;
    
    glm::vec3 position;
    glm::vec2 rotation;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;

    GPUCameraData gpuData;
    std::shared_ptr<Resource> cameraDataBuffer;
    std::shared_ptr<View> cameraDataView;
    BindingDesc cameraDataDescFragment;
    BindingDesc cameraDataDescMesh;
    BindingDesc cameraDataDescAmplification;
    BindingDesc cameraDataDescVertex;
    BindingDesc cameraDataDescCompute;
    BindKey cameraDataKeyFragment = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };
    BindKey cameraDataKeyMesh = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };
    BindKey cameraDataKeyAmplification = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };
    BindKey cameraDataKeyVertex = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };
    BindKey cameraDataKeyCompute = { ShaderType::kUnknown, ViewType::kUnknown, 0, 0, 0, UINT32_MAX };

    Camera(std::shared_ptr<Device> device, AppBox& app);

    void UpdateCamera(const AppSize& size);
};