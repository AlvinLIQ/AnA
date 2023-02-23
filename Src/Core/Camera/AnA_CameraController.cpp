#include "Headers/AnA_CameraController.hpp"

using namespace AnA;
using namespace AnA::Camera;

const float moveStep = 1.0f;
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
    glm::vec3 position;
    int posIndex = param->id / 2;
    position[(posIndex + 1) % 3] = 0;
    position[(posIndex + 2) % 3] = 0;
    position[posIndex] = (param->id & 1 ? -moveStep : moveStep) * speedRatio;
    param->aCamera.AddViewOffset(position);
}