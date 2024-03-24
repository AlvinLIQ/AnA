#pragma once
#include "../../Headers/Window.hpp"
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
            int action;
        };
        struct CursorPosition
        {
            double x;
            double y;
        };
        struct CursorConfig
        {
            void* param;
            void(*callBack)(void* pParam, CursorPosition &curPos);
            int action;
        };
        enum InputProfileFlags
        {
            None = 0, HideCursor = 1, RawMotion = 2
        };
        struct InputProfile
        {
            uint32_t flag;
            std::vector<KeyMapConfig> keyMapConfigs;
            std::vector<CursorConfig> cursorConfigs;
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
            void SetActiveProfile(int profileIndex);
            
            bool CheckAndRunCallbacks();
        private:
            Window& aWindow;
            static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            int activeProfileIndex = 0;
            std::vector<InputProfile> inputProfiles{1};

            CursorPosition curPos, prevPos;
        };
    }
}