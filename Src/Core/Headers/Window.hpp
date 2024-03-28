#pragma once

#include <stdexcept>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Instance.hpp"

#define DEFAULT_WINDOW_WIDTH 1360
#define DEFAULT_WINDOW_HEIGHT 768

namespace AnA
{
    class Window
    {
    public:
        Window();
        ~Window();

        int Init();
        void StartLoop();
        void CloseLoop();

        void CreateWindowSurface(Instance* aInstance)
        {
            if (glfwCreateWindowSurface(aInstance->GetInstance(), window, nullptr, &surface) != VK_SUCCESS)
                throw std::runtime_error("Failed to create window surface!");
        }

        GLFWwindow* GetGLFWwindow()
        {
            return window;
        }

        VkSurfaceKHR &GetSurface()
        {
            return surface;
        }

        int Width = DEFAULT_WINDOW_WIDTH;
        int Height = DEFAULT_WINDOW_HEIGHT;

        bool FramebufferResized = false;
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        GLFWwindow* window;
        VkSurfaceKHR surface;


        void mainLoop();
    };
}
