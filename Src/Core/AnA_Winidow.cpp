#include "Headers/AnA_Window.h"
#include <GLFW/glfw3.h>
#include <thread>

using namespace AnA;

AnA_Window::AnA_Window()
{
    
}

AnA_Window::~AnA_Window()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

int AnA_Window::Init(void* userPointer, GLFWframebuffersizefun framebufferResizeCallback)
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(800, 600, "AnA", NULL, NULL);
    if (!window)
        return -1;

    glfwSetWindowUserPointer(window, userPointer);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        
    glfwMakeContextCurrent(window);
    return 0;
}

#ifdef _AnA_async
std::thread loopThread;
void AnA_Window::StartLoop()
{
    loopThread = std::thread(&AnA_Window::mainLoop);
    loopThread.detach();
}

void AnA_Window::CloseLoop()
{
    glfwSetWindowShouldClose(window, GL_TRUE);
    if (loopThread.joinable())
    {
        loopThread.join();
    }
}
#else
void AnA_Window::StartLoop()
{
    mainLoop();
}
#endif

void AnA_Window::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}
