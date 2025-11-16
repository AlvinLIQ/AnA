#include "Headers/CameraController.hpp"
#include "Headers/Camera.hpp"

#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>

using namespace AnA;
using namespace AnA::Cameras;

#define MOVEMENTSIZE 6
#define ANA_MOVE_LEFTRIGHT 0
#define ANA_MOVE_UPDOWN 1
#define ANA_MOVE_FORWARDBACK 2

const int keyCodes[] = {GLFW_KEY_D, GLFW_KEY_A, GLFW_KEY_C,GLFW_KEY_SPACE, GLFW_KEY_S, GLFW_KEY_W};

void CameraController::GetCameraKeyMapConfigs(std::vector<Input::KeyMapConfig> &configs)
{
    Input::KeyMapConfig config;

    config.callBack = reinterpret_cast<Input::RegularCallBack>(CameraController::Move);

    for (size_t i = 0; i < MOVEMENTSIZE; i++)
    {
        config.keyCode = keyCodes[i];
        config.param = &movements[i];
        configs.push_back(config);
    }
}

void CameraController::GetCameraCursorConfigs(std::vector<Input::CursorConfig> &configs)
{
    Input::CursorConfig config;
    config.param = &aCamera;
    config.callBack = reinterpret_cast<Input::CursorCallBack>(CameraController::CursorMoved);
    configs.push_back(config);
}

void CameraController::GetInputProfile(Input::InputProfile& inputProfile)
{
    inputProfile.flag = Input::InputProfileFlags::HideCursor | Input::InputProfileFlags::RawMotion;
    GetCameraKeyMapConfigs(inputProfile.keyMapConfigs);
    GetCameraCursorConfigs(inputProfile.cursorConfigs);
    inputProfile.param = &aCamera;
    inputProfile.callback = [](void* camera)
    {
        static_cast<Camera*>(camera)->UpdateViewMatrix();
    };
}

CameraController::CameraController(Camera &mCamera) : aCamera {mCamera}
{
    CameraController::CameraCallbackParam param{aCamera, 0};
    for (int i = 0; i < MOVEMENTSIZE; i++)
    {
        param.id = i;
        movements.push_back(param);
    }
}

void CameraController::Move(CameraController::CameraCallbackParam* param)
{
    int posIndex = param->id >> 1;
    auto &roY = param->aCamera.CameraTransform.rotation.y;
    glm::vec3 moveDirection;
    if (posIndex == ANA_MOVE_LEFTRIGHT)
    {
        moveDirection = {cosf(roY), 0.f, -sinf(roY)};
    }
    else if (posIndex == ANA_MOVE_UPDOWN)
    {
        moveDirection = {0.f, 1.f, 0.f};
    }
    else if (posIndex == ANA_MOVE_FORWARDBACK)
    {
        moveDirection = {sinf(roY), 0.f, cosf(roY)};
    }
    param->aCamera.offset -= param->id & 1 ? -moveDirection : moveDirection;
}

void CameraController::Rotate(CameraController::CameraCallbackParam* param)
{
    int posIndex = param->id >> 1;
    param->aCamera.CameraTransform.rotation[posIndex] -= (param->id & 1 ? -rotateStep : rotateStep) * param->aCamera.GetRotateSpeed() * 6.283f;
}

void CameraController::CursorMoved(Camera* camera, CursorPosition &duration, int )
{
    const float rotateSpeed = camera->GetSpeedRatio() * camera->GetRotateSpeed() * 6.283f * 80.f;
    camera->CameraTransform.rotation.y = glm::mod(camera->CameraTransform.rotation.y - static_cast<float>(duration.x) * rotateSpeed, glm::two_pi<float>());
    camera->CameraTransform.rotation.x += static_cast<float>(duration.y) * rotateSpeed;

    const float yLock = .2f * glm::two_pi<float>();
    camera->CameraTransform.rotation.x = glm::clamp(camera->CameraTransform.rotation.x, -yLock, yLock);
}