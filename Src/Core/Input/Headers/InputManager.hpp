#pragma once
#include "../../Headers/Window.hpp"
#include "../../Headers/Types.hpp"
#include <vector>

namespace AnA
{
    namespace Input
    {
        typedef void(*RegularCallBack)(void* param);
        struct KeyMapConfig
        {
            void* param;
            RegularCallBack callBack;
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
        typedef void(*CharacterCallBack)(uint32_t ch);
        struct CharacterConfig
        { 
            CharacterCallBack callBack;
            uint32_t ch;
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
            std::vector<CharacterConfig> characterConfigs;
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
                return inputProfiles[static_cast<size_t>(activeProfileIndex)];
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
            void SetActiveProfile(uint32_t profileIndex);
            void ProcessProfileFlag(uint32_t profileFlag);

            void Check();
        private:
            Window& aWindow;
            static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void characterCallback(GLFWwindow* window, uint32_t ch);
            int activeProfileIndex = 0;
            std::vector<InputProfile> inputProfiles{1};
            CursorPosition cursorPos;
            CursorPosition curPos, prevPos;
            void checkProfile(Input::InputProfile& inputProfile);
        };
    }
}