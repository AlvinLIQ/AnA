#include "Headers/AnA_Window.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
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

int AnA_Window::Init()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "AnA", NULL, NULL);
    if (!window)
        return -1;

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, AnA_Window::FramebufferResizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

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

void AnA_Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto aWindow = reinterpret_cast<AnA_Window*>(glfwGetWindowUserPointer(window));
    aWindow->FramebufferResized = true;
    aWindow->Width = width;
    aWindow->Height = height;
}