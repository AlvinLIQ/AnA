#include "Headers/AnA_InputManager.hpp"
#include <GLFW/glfw3.h>

using namespace AnA;
using namespace AnA::Input;

AnA_InputManager *_aInputManager;

AnA_InputManager::AnA_InputManager(AnA_Window *&mWindow) : aWindow {mWindow}
{
    _aInputManager = this;
/*
    auto &window = aWindow->GetGLFWwindow();
    glfwSetKeyCallback(window, AnA_InputManager::keyCallback);*/
}

void AnA_InputManager::CheckAndRunCallbacks()
{
    auto &window = aWindow->GetGLFWwindow();
    auto &keyMapConfigs = _aInputManager->GetKeyMapConfigs();
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.callBack != nullptr && glfwGetKey(window, keyMapConfig.keyCode) == GLFW_PRESS)
            keyMapConfig.callBack();
    }
}

void AnA_InputManager::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto &keyMapConfigs = _aInputManager->GetKeyMapConfigs();
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.keyCode == key && keyMapConfig.callBack != nullptr)
            keyMapConfig.callBack();
    }
}