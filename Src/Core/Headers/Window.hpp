#pragma once

#include <stdexcept>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "Types.hpp"

#define DEFAULT_WINDOW_WIDTH 1366
#define DEFAULT_WINDOW_HEIGHT 768

namespace AnA
{
    class Window
    {
    public:
        Window();
        ~Window();

        void StartLoop();
        void CloseLoop();

        void CreateWindowSurface(Instance* instance)
        {
            if (glfwCreateWindowSurface(instance->GetInstance(), window, nullptr, &surface) != VK_SUCCESS)
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

        Int32 Width = DEFAULT_WINDOW_WIDTH;
        Int32 Height = DEFAULT_WINDOW_HEIGHT;

        bool FramebufferResized = false;
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        GLFWwindow* window;
        VkSurfaceKHR surface{VK_NULL_HANDLE};

        int init();
        void mainLoop();
    };
}
