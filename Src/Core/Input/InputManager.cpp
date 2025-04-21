#include "Headers/InputManager.hpp"
#include <GLFW/glfw3.h>

using namespace AnA;
using namespace AnA::Input;

InputManager* _aInputManager;

InputManager::InputManager(Window& mWindow) : aWindow {mWindow}
{
    _aInputManager = this;

    auto window = aWindow.GetGLFWwindow();
    glfwSetKeyCallback(window, InputManager::keyCallback);
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

void InputManager::SetActiveProfile(uint32_t profileIndex)
{
    if (profileIndex < static_cast<uint32_t>(inputProfiles.size()))
    {
        activeProfileIndex = profileIndex;
        ProcessProfileFlag(inputProfiles[activeProfileIndex].flag);
    }
}


void InputManager::ProcessProfileFlag(uint32_t profileFlag)
{
    auto window = aWindow.GetGLFWwindow();
    if (profileFlag & InputProfileFlags::HideCursor)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, profileFlag & InputProfileFlags::RawMotion && glfwRawMouseMotionSupported());
}

void InputManager::Check()
{
    checkProfile(inputProfiles[activeProfileIndex]);
    //checkProfile(GlobalProfile);
}

void InputManager::keyCallback(GLFWwindow* , int key, int , int action, int )
{
    auto &keyMapConfigs = _aInputManager->GlobalProfile.keyMapConfigs;
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.keyCode == key && keyMapConfig.callBack != nullptr && keyMapConfig.action == action)
            keyMapConfig.callBack(keyMapConfig.param);
    }
}

void InputManager::checkProfile(Input::InputProfile& inputProfile)
{
    if (inputProfile.flag & InputProfileFlags::Disabled)
        return;
    auto window = aWindow.GetGLFWwindow();
    glfwGetCursorPos(window, &curPos.x, &curPos.y);
    CursorPosition duration = {(curPos.x - prevPos.x) / aWindow.Width.As<double>(), (curPos.y - prevPos.y) / aWindow.Height.As<double>()};
    cursorPos = {curPos.x / (double)aWindow.Width, curPos.y / (double)aWindow.Height};
    auto &cursorConfigs = inputProfile.cursorConfigs;
    int leftButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    for (auto &cursorConfig : cursorConfigs)
    {
        if (cursorConfig.callBack != nullptr)
            cursorConfig.callBack(cursorConfig.param, inputProfile.flag & InputProfileFlags::RawMotion ? duration : cursorPos, leftButton);
    }
    prevPos = curPos;

    auto &keyMapConfigs = inputProfile.keyMapConfigs;
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.callBack != nullptr && glfwGetKey(window, keyMapConfig.keyCode) == keyMapConfig.action)
            keyMapConfig.callBack(keyMapConfig.param);
    }
    //glfwSetCursorPos(window, centerX, centerY);
    if (inputProfile.callback != nullptr && keyMapConfigs.size() + cursorConfigs.size() > 0)
    {
        inputProfile.callback(inputProfile.param);
    }
}
