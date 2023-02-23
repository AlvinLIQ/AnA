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
            AnA_InputManager(AnA_Window *&mWindow);
            ~AnA_InputManager();

            std::vector<KeyMapConfig> &GetKeyMapConfigs()
            {
                return keyMapConfigs;
            }
            bool CheckAndRunCallbacks();
        private:
            AnA_Window *&aWindow;
            std::vector<KeyMapConfig> keyMapConfigs;
            static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        };
    }
}