#pragma once
#include "../../Headers/Window.hpp"
#include "../../Headers/Types.hpp"
#include <vector>

namespace AnA
{
    namespace Input
    {
        struct KeyMapConfig
        {
            void* param;
            void(*callBack)(void* param);
            int keyCode;
            int action = GLFW_PRESS;
        };
        typedef void(*CursorCallBack)(void* pParam, CursorPosition &curPos, int leftButtonAction);
        struct CursorConfig
        {
            void* param;
            CursorCallBack callBack;
            int action;
        };
        enum InputProfileFlags
        {
            None = 0, HideCursor = 1, RawMotion = 2, Disabled = 4
        };
        struct InputProfile
        {
            uint32_t flag;
            std::vector<KeyMapConfig> keyMapConfigs;
            std::vector<CursorConfig> cursorConfigs;
            void* param;
            //only call when there's a config
            void(*callback)(void* param) = nullptr;
        };
        class InputManager
        {
        public:
            InputManager(Window& mWindow);
            ~InputManager();

            InputProfile& GetActiveProfile()
            {
                return inputProfiles[activeProfileIndex];
            }

            std::vector<InputProfile>& GetProfiles()
            {
                return inputProfiles;
            }

            CursorPosition& GetCursorPosition()
            {
                return cursorPos;
            }
            InputProfile GlobalProfile{};
            void SetActiveProfile(int profileIndex);
            void ProcessProfileFlag(uint32_t profileFlag);

            void Check();
        private:
            Window& aWindow;
            static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            int activeProfileIndex = 0;
            std::vector<InputProfile> inputProfiles{1};
            CursorPosition cursorPos;
            CursorPosition curPos, prevPos;
            void checkProfile(Input::InputProfile& inputProfile);
        };
    }
}