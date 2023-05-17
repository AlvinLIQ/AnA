#pragma once
#include "../../Headers/Window.hpp"
#include <vector>

namespace AnA
{
    namespace Input
    {
        class InputManager
        {
        public:
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
            InputManager(Window& mWindow);
            ~InputManager();

            std::vector<KeyMapConfig> &GetKeyMapConfigs()
            {
                return keyMapConfigs;
            }
            std::vector<CursorConfig> &GetCursorConfigs()
            {
                return cursorConfigs;
            }
            
            bool CheckAndRunCallbacks();
        private:
            Window& aWindow;
            std::vector<KeyMapConfig> keyMapConfigs;
            static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

            std::vector<CursorConfig> cursorConfigs;

            CursorPosition curPos, prevPos;
        };
    }
}