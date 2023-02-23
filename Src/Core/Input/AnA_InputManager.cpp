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

AnA_InputManager::~AnA_InputManager()
{
    keyMapConfigs.clear();
}

bool AnA_InputManager::CheckAndRunCallbacks()
{
    auto &window = aWindow->GetGLFWwindow();
    auto &keyMapConfigs = _aInputManager->GetKeyMapConfigs();
    size_t configSize = keyMapConfigs.size();
    for (size_t i = 0; i < configSize; i++)
    {
        auto &keyMapConfig = keyMapConfigs[i];
        if (keyMapConfig.callBack != nullptr && glfwGetKey(window, keyMapConfig.keyCode) == GLFW_PRESS)
            keyMapConfig.callBack(keyMapConfig.param);
    }

    return configSize > 0;
}

void AnA_InputManager::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto &keyMapConfigs = _aInputManager->GetKeyMapConfigs();
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.keyCode == key && keyMapConfig.callBack != nullptr)
            keyMapConfig.callBack(keyMapConfig.param);
    }
}