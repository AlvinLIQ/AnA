#include "Headers/AnA_CameraController.hpp"
#include "Headers/AnA_Camera.hpp"

using namespace AnA;
using namespace AnA::Camera;

const float moveStep = 1.0f;
const float rotateStep = 0.8f;
float speedRatio = 1.0f;

#define MOVEMENTSIZE 6

const int keyCodes[] = {GLFW_KEY_D, GLFW_KEY_A, GLFW_KEY_C, GLFW_KEY_SPACE, GLFW_KEY_W, GLFW_KEY_S};

void AnA_CameraController::GetCameraKeyMapConfigs(std::vector<Input::AnA_InputManager::KeyMapConfig> &configs)
{
    Input::AnA_InputManager::KeyMapConfig config;

    config.callBack = (void(*)(void*))AnA_CameraController::Move;

    for (int i = 0; i < MOVEMENTSIZE; i++)
    {
        config.keyCode = keyCodes[i];
        config.param = &movements[i];
        configs.push_back(config);
    }
}

void AnA_CameraController::GetCameraCursorConfigs(std::vector<Input::AnA_InputManager::CursorConfig> &configs)
{
    Input::AnA_InputManager::CursorConfig config;
    config.param = &aCamera;
    config.callBack = (void(*)(void*, Input::AnA_InputManager::CursorPosition&))AnA_CameraController::CursorMoved;
    configs.push_back(config);
}

AnA_CameraController::AnA_CameraController(AnA_Camera &mCamera) : aCamera {mCamera}
{
    AnA_CameraController::CameraCallbackParam param{aCamera};
    for (int i = 0; i < MOVEMENTSIZE; i++)
    {
        param.id = i;
        movements.push_back(param);
    }
}

void AnA_CameraController::SetSpeedRatio(float ratio)
{
    speedRatio = ratio;
}

void AnA_CameraController::Move(AnA_CameraController::CameraCallbackParam *param)
{
    int posIndex = param->id / 2;
    param->aCamera.CameraTransform.translation[posIndex] -= (param->id & 1 ? -moveStep : moveStep) * speedRatio;
}

void AnA_CameraController::Rotate(AnA_CameraController::CameraCallbackParam *param)
{
    int posIndex = param->id / 2;
    param->aCamera.CameraTransform.rotation[posIndex] -= (param->id & 1 ? -rotateStep : rotateStep) * speedRatio * 6.283;
}

void AnA_CameraController::CursorMoved(AnA_Camera *camera, Input::AnA_InputManager::CursorPosition &duration)
{
    const float rotateSpeed = speedRatio * 6.283 * 18.;
    camera->CameraTransform.rotation.y -= duration.x * rotateSpeed;
    camera->CameraTransform.rotation.x -= duration.y * rotateSpeed;
    if (camera->CameraTransform.rotation.x > .25 * 6.283)
        camera->CameraTransform.rotation.x = .25 * 6.283;
    else if (camera->CameraTransform.rotation.x < -.25 * 6.283)
        camera->CameraTransform.rotation.x = -.25 * 6.283;
}