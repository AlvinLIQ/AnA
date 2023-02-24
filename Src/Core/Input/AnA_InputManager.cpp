#include "Headers/AnA_InputManager.hpp"
#include <GLFW/glfw3.h>
#include <cstdio>

using namespace AnA;
using namespace AnA::Input;

AnA_InputManager *_aInputManager;

AnA_InputManager::AnA_InputManager(AnA_Window *&mWindow) : aWindow {mWindow}
{
    _aInputManager = this;
/*
    auto &window = aWindow->GetGLFWwindow();
    glfwSetKeyCallback(window, AnA_InputManager::keyCallback);*/
    glfwGetCursorPos(aWindow->GetGLFWwindow(), &prevPos.x, &prevPos.y);
}

AnA_InputManager::~AnA_InputManager()
{
    keyMapConfigs.clear();
}

bool AnA_InputManager::CheckAndRunCallbacks()
{
    auto &window = aWindow->GetGLFWwindow();
    glfwGetCursorPos(window, &curPos.x, &curPos.y);
    CursorPosition duration = {(curPos.x - prevPos.x) / (double)aWindow->Width, (curPos.y - prevPos.y) / (double)aWindow->Height};
    auto &cursorConfigs = _aInputManager->GetCursorConfigs();
    for (auto &cursorConfig : cursorConfigs)
    {
        if (cursorConfig.callBack != nullptr)
            cursorConfig.callBack(cursorConfig.param, duration);
    }
    prevPos = curPos;

    auto &keyMapConfigs = _aInputManager->GetKeyMapConfigs();
    size_t configSize = keyMapConfigs.size();
    for (size_t i = 0; i < configSize; i++)
    {
        auto &keyMapConfig = keyMapConfigs[i];
        if (keyMapConfig.callBack != nullptr && glfwGetKey(window, keyMapConfig.keyCode) == GLFW_PRESS)
            keyMapConfig.callBack(keyMapConfig.param);
    }
    //glfwSetCursorPos(window, centerX, centerY);
    return configSize + cursorConfigs.size() > 0;
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