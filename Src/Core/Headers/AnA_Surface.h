#pragma once

#include <stdexcept>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace AnA
{
    class AnA_Surface
    {
    public:
        AnA_Surface(VkInstance mInstance, GLFWwindow* window)
        {
            instance = mInstance;
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
                throw std::runtime_error("Couldn't create window surface!");
        }
        ~AnA_Surface()
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }

        VkSurfaceKHR GetSurface()
        {
            return surface;
        }
    private:
        VkSurfaceKHR surface;
        VkInstance instance;
    };
}
