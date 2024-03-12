#include "Headers/CameraController.hpp"
#include "Headers/Camera.hpp"

#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>

using namespace AnA;
using namespace AnA::Cameras;

#define MOVEMENTSIZE 6

const int keyCodes[] = {GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_C, GLFW_KEY_S, GLFW_KEY_W};

void CameraController::GetCameraKeyMapConfigs(std::vector<Input::InputManager::KeyMapConfig> &configs)
{
    Input::InputManager::KeyMapConfig config;

    config.callBack = (void(*)(void*))CameraController::Move;

    for (int i = 0; i < MOVEMENTSIZE; i++)
    {
        config.keyCode = keyCodes[i];
        config.param = &movements[i];
        configs.push_back(config);
    }
}

void CameraController::GetCameraCursorConfigs(std::vector<Input::InputManager::CursorConfig> &configs)
{
    Input::InputManager::CursorConfig config;
    config.param = &aCamera;
    config.callBack = (void(*)(void*, Input::InputManager::CursorPosition&))CameraController::CursorMoved;
    configs.push_back(config);
}

CameraController::CameraController(Camera &mCamera) : aCamera {mCamera}
{
    CameraController::CameraCallbackParam param{aCamera};
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
    if (posIndex == 0)
    {
        moveDirection = {cos(roY), 0.f, -sin(roY)};
    }
    else if (posIndex == 1)
    {
        moveDirection = {0.f, 1.f, 0.f};
    }
    else if (posIndex == 2)
    {
        moveDirection = {sin(roY), 0.f, cos(roY)};
    }
    param->aCamera.offset -= param->id & 1 ? -moveDirection : moveDirection;
}

void CameraController::Rotate(CameraController::CameraCallbackParam* param)
{
    int posIndex = param->id >> 1;
    param->aCamera.CameraTransform.rotation[posIndex] -= (param->id & 1 ? -rotateStep : rotateStep) * param->aCamera.GetSpeedRatio() * 6.283;
}

void CameraController::CursorMoved(Camera* camera, Input::InputManager::CursorPosition &duration)
{
    const float rotateSpeed = camera->GetSpeedRatio() * 6.283 * 80.;
    camera->CameraTransform.rotation.y = glm::mod(camera->CameraTransform.rotation.y + (float)duration.x * rotateSpeed, glm::two_pi<float>());
    camera->CameraTransform.rotation.x -= (float)duration.y * rotateSpeed;

    const float yLock = .2f * glm::two_pi<float>();
    camera->CameraTransform.rotation.x = glm::clamp(camera->CameraTransform.rotation.x, -yLock, yLock);
}