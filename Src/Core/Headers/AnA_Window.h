#ifndef _ANA_WINDOW_H
#define _ANA_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace AnA
{
    class AnA_Window
    {
    public:
        AnA_Window();
        ~AnA_Window();

        int Init(void* userPointer, GLFWframebuffersizefun framebufferResizeCallback);
        void StartLoop();
        void CloseLoop();

        GLFWwindow *&GetGLFWwindow()
        {
            return window;
        }
    private:
        GLFWwindow* window;

        void mainLoop();
    };
}

#endif