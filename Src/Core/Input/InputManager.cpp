#include "Headers/InputManager.hpp"
#include <GLFW/glfw3.h>

using namespace AnA;
using namespace AnA::Input;

InputManager* _aInputManager;

InputManager::InputManager(Window& mWindow) : aWindow {mWindow}
{
    _aInputManager = this;
/*
    auto &window = aWindow.GetGLFWwindow();
    glfwSetKeyCallback(window, InputManager::keyCallback);*/
    glfwGetCursorPos(aWindow.GetGLFWwindow(), &prevPos.x, &prevPos.y);
}

InputManager::~InputManager()
{
    for (auto& inputProfile : inputProfiles)
    {
        inputProfile.keyMapConfigs.clear();
        inputProfile.cursorConfigs.clear();
    }
}

void InputManager::SetActiveProfile(int profileIndex)
{
    if (profileIndex < inputProfiles.size())
    {
        activeProfileIndex = profileIndex;
        auto window = aWindow.GetGLFWwindow();
        auto& profileFlag = inputProfiles[profileIndex].flag;
        if (profileFlag & InputProfileFlags::HideCursor)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, profileFlag & InputProfileFlags::RawMotion && glfwRawMouseMotionSupported());
    }
}

bool InputManager::CheckAndRunCallbacks()
{
    auto window = aWindow.GetGLFWwindow();
    glfwGetCursorPos(window, &curPos.x, &curPos.y);
    CursorPosition duration = {(curPos.x - prevPos.x) / (double)aWindow.Width, (curPos.y - prevPos.y) / (double)aWindow.Height};
    auto &cursorConfigs = inputProfiles[activeProfileIndex].cursorConfigs;
    for (auto &cursorConfig : cursorConfigs)
    {
        if (cursorConfig.callBack != nullptr)
            cursorConfig.callBack(cursorConfig.param, duration);
    }
    prevPos = curPos;

    auto &keyMapConfigs = inputProfiles[activeProfileIndex].keyMapConfigs;
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
    auto &keyMapConfigs = _aInputManager->GetActiveProfile().keyMapConfigs;
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.keyCode == key && keyMapConfig.callBack != nullptr)
            keyMapConfig.callBack(keyMapConfig.param);
    }
}