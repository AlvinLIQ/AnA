#include "Headers/InputManager.hpp"

using namespace AnA;
using namespace AnA::Input;

InputManager* _aInputManager;

InputManager::InputManager(Window& mWindow) : aWindow {mWindow}
{
    _aInputManager = this;

    aWindow.KeyCallback = keyCallback;
    aWindow.ScrollCallback = scrollCallback;
    aWindow.TextCallback = textCallback;
    //glfwSetKeyCallback(window, InputManager::keyCallback);
    //glfwSetCharCallback(window, InputManager::characterCallback);
    //glfwSetScrollCallback(window, InputManager::scrollCallback);
    //glfwGetCursorPos(aWindow.GetSDLWindow(), &prevPos.x, &prevPos.y);
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
    if (profileFlag & InputProfileFlags::HideCursor)
        aWindow.HideCursor();
    else
        aWindow.ShowCursor();
    aWindow.SetRawMotion(profileFlag & InputProfileFlags::RawMotion);
}

void InputManager::Check()
{
    checkProfile(inputProfiles[activeProfileIndex]);
    checkCursorConfigs(GlobalProfile);
    //checkProfile(GlobalProfile);
    prevRelPos = curRelPos;
}

void InputManager::keyCallback(int scancode, int action)
{
    auto &keyMapConfigs = _aInputManager->GlobalProfile.keyMapConfigs;
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.keyCode == scancode && keyMapConfig.callback != nullptr && keyMapConfig.action == action)
            keyMapConfig.callback(keyMapConfig.param);
    }
    auto& activeProfile = _aInputManager->GetActiveProfile();
    for (auto &keyMapConfig : activeProfile.opKeyMapConfigs)
    {
        if (keyMapConfig.keyCode == scancode && keyMapConfig.callback != nullptr && keyMapConfig.action == action)
            keyMapConfig.callback(keyMapConfig.param);
    }
}
/*
void InputManager::characterCallback(GLFWwindow* , uint32_t ch)
{
    auto &characterConfigs = _aInputManager->GetActiveProfile().characterConfigs;
    for (auto& characterConfig : characterConfigs)
    {
        characterConfig.callback(ch);
    }
}
*/
void InputManager::scrollCallback(float dx, float dy)
{
    auto &scrollConfigs = _aInputManager->GetActiveProfile().scrollConfigs;
    for (auto& scrollConfig : scrollConfigs)
    {
        scrollConfig.callback(dx, dy);
    }
}

void InputManager::textCallback(const char* text)
{
    auto &textConfigs = _aInputManager->GetActiveProfile().textConfigs;
    for (auto& textConfig : textConfigs)
    {
        textConfig.callback(text);
    }
}


void InputManager::checkCursorConfigs(Input::InputProfile& inputProfile)
{
    if (inputProfile.flag & InputProfileFlags::CursorDisabled)
        return;

    curPos = aWindow.GetCursorPos();

    curRelPos = aWindow.GetCursorRelativePos();
    curArgs.duration = {(curRelPos.x) / aWindow.Width.As<double>(), (curRelPos.y) / aWindow.Height.As<double>()};
    curArgs.pos = {curPos.x / (double)aWindow.Width, curPos.y / (double)aWindow.Height};
    auto &cursorConfigs = inputProfile.cursorConfigs;
    int leftButton = aWindow.GetMouseLeftButton();
    for (auto &cursorConfig : cursorConfigs)
    {
        if (cursorConfig.callback != nullptr)
            cursorConfig.callback(cursorConfig.param, curArgs, leftButton);
    }
}

void InputManager::checkProfile(Input::InputProfile& inputProfile)
{
    if (inputProfile.flag & InputProfileFlags::Disabled)
        return;

    checkCursorConfigs(inputProfile);
    //auto window = aWindow.GetSDLWindow();
    auto &keyMapConfigs = inputProfile.keyMapConfigs;

    auto sdlKeyStates = SDL_GetKeyboardState(NULL);
    for (auto &keyMapConfig : keyMapConfigs)
    {
        if (keyMapConfig.callback != nullptr && sdlKeyStates[keyMapConfig.keyCode] == 1)
        {
            keyMapConfig.callback(keyMapConfig.param);
        }
    }
    //glfwSetCursorPos(window, centerX, centerY);
    if (inputProfile.callback != nullptr && keyMapConfigs.size() + inputProfile.cursorConfigs.size() > 0)
    {
        inputProfile.callback(inputProfile.param);
    }
}
