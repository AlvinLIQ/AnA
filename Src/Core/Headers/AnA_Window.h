#pragma once

#include <stdexcept>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "AnA_Instance.h"


namespace AnA
{
    class AnA_Window
    {
    public:
        AnA_Window();
        ~AnA_Window();

        int Init();
        void StartLoop();
        void CloseLoop();

        void CreateWindowSurface(AnA_Instance* aInstance)
        {
            if (glfwCreateWindowSurface(aInstance->GetInstance(), window, nullptr, &surface) != VK_SUCCESS)
                throw std::runtime_error("Failed to create window surface!");
        }

        GLFWwindow *&GetGLFWwindow()
        {
            return window;
        }

        VkSurfaceKHR &GetSurface()
        {
            return surface;
        }

        bool FramebufferResized = false;
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        GLFWwindow* window;
        VkSurfaceKHR surface;

        void mainLoop();
    };
}
