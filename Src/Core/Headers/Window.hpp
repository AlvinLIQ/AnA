#pragma once

#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_vulkan.h>

#include "Instance.hpp"
#include "Types.hpp"

#define DEFAULT_WINDOW_WIDTH 1366
#define DEFAULT_WINDOW_HEIGHT 768

namespace AnA
{
    class Window
    {
    public:
        Window(const char* title);
        ~Window();

        void StartLoop();
        void CloseLoop();

        void CreateWindowSurface(Instance* instance)
        {
            if (!SDL_Vulkan_CreateSurface(window, instance->GetInstance(), nullptr, &surface))
                throw std::runtime_error("Failed to create window surface!");
        }

        SDL_Window* GetSDLWindow()
        {
            return window;
        }

        VkSurfaceKHR &GetSurface()
        {
            return surface;
        }

        void PollEvents();
        void WaitEvent()
        {
            SDL_WaitEvent(&event);
        }
        bool GetExitSignal()
        {
            return exitSignal;
        }

        void SetTitle(const char* title)
        {
            SDL_SetWindowTitle(window, title);
        }

        CursorPosition GetCursorPos()
        {
            return cursorPos;
        }
        CursorPosition GetCursorRelativePos()
        {
            float x, y;
            SDL_GetRelativeMouseState(&x, &y);
            return {x, y};
        }
        void ShowCursor()
        {
            SDL_ShowCursor();
        }
        void HideCursor()
        {
            SDL_HideCursor();
        }
        void SetRawMotion(bool enable)
        {
            SDL_SetWindowRelativeMouseMode(window, enable);
        }

        void GetScale(float* x, float* y)
        {
            *x = SDL_GetWindowDisplayScale(window);
            *y = *x;
        }

        int GetMouseLeftButton()
        {
            return leftButton;
        }

        int GetFingerCount()
        {
            return fingerCount;
        }

        void Exit()
        {
            exitSignal = true;
        }

        Int32 Width = DEFAULT_WINDOW_WIDTH;
        Int32 Height = DEFAULT_WINDOW_HEIGHT;

        bool FramebufferResized = false;
        void (*KeyCallback)(int scancode, int action) = nullptr;
        void (*ScrollCallback)(float dx, float dy) = nullptr;
        //static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        SDL_Window* window;
        SDL_Event event;
        int fingerCount;
        float prevFingerDistance = 0.0f;
        VkSurfaceKHR surface{VK_NULL_HANDLE};
        bool exitSignal = false;
        CursorPosition cursorPos{};

        int leftButton = 0;

        int init(const char* name);
        void mainLoop();
    };
}
