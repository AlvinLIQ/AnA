#include "Headers/InputManager.hpp"
#include <GLFW/glfw3.h>
#include <cstdio>

using namespace AnA;
using namespace AnA::Input;

InputManager* _aInputManager;

InputManager::InputManager(Window*& mWindow) : aWindow {mWindow}
{
    _aInputManager = this;
/*
    auto &window = aWindow->GetGLFWwindow();
    glfwSetKeyCallback(window, InputManager::keyCallback);*/
    glfwGetCursorPos(aWindow->GetGLFWwindow(), &prevPos.x, &prevPos.y);
}

InputManager::~InputManager()
{
    keyMapConfigs.clear();
    cursorConfigs.clear();
}

bool InputManager::CheckAndRunCallbacks()
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

    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.callBack != nullptr && glfwGetKey(window, keyMapConfig.keyCode) == GLFW_PRESS)
            keyMapConfig.callBack(keyMapConfig.param);
    }
    //glfwSetCursorPos(window, centerX, centerY);
    return keyMapConfigs.size() + cursorConfigs.size() > 0;
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto &keyMapConfigs = _aInputManager->GetKeyMapConfigs();
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.keyCode == key && keyMapConfig.callBack != nullptr)
            keyMapConfig.callBack(keyMapConfig.param);
    }
}