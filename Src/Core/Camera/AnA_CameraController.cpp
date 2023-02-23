#include "Headers/AnA_CameraController.hpp"

using namespace AnA;
using namespace AnA::Camera;

const float moveStep = 0.01;

void AnA_CameraController::GetCameraKeyMapConfigs(std::vector<Input::AnA_InputManager::KeyMapConfig> &configs)
{
    Input::AnA_InputManager::KeyMapConfig config;
    config.keyCode = GLFW_KEY_W;

    config.param = &movements[0];
    config.callBack = (void(*)(void*))AnA_CameraController::Move;
    configs.push_back(config);

    config.keyCode = GLFW_KEY_S;
    config.param = &movements[1];
    configs.push_back(config);

    config.keyCode = GLFW_KEY_A;
    config.param = &movements[2];
    configs.push_back(config);

    config.keyCode = GLFW_KEY_D;
    config.param = &movements[3];
    configs.push_back(config);
}

AnA_CameraController::AnA_CameraController(AnA_Camera &mCamera) : aCamera {mCamera}
{
    AnA_CameraController::CameraCallbackParam param{aCamera, glm::vec3(0.f, 0.f, moveStep)};
    movements.push_back(param);
    param.position = glm::vec3(0.f, 0.f, -moveStep);
    movements.push_back(param);
    param.position = glm::vec3(-moveStep, 0.f, 0.f);
    movements.push_back(param);
    param.position = glm::vec3(moveStep, 0.f, 0.f);
    movements.push_back(param);
}

void AnA_CameraController::Move(AnA_CameraController::CameraCallbackParam *param)
{
    param->aCamera.AddViewOffset(param->position);
}