#pragma once
#include "../../Headers/AnA_Window.hpp"
#include <vector>

namespace AnA
{
    namespace Input
    {
        class AnA_InputManager
        {
        public:
            struct KeyMapConfig
            {
                void *param;
                void(*callBack)(void *param);
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
                void *param;
                void(*callBack)(void* pParam, CursorPosition &curPos);
                int action;
            };
            AnA_InputManager(AnA_Window *&mWindow);
            ~AnA_InputManager();

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
            AnA_Window *&aWindow;
            std::vector<KeyMapConfig> keyMapConfigs;
            static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

            std::vector<CursorConfig> cursorConfigs;

            CursorPosition curPos, prevPos;
        };
    }
}