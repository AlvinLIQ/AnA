#include "Headers/Window.hpp"
#include <GLFW/glfw3.h>

using namespace AnA;

Window::Window()
{
}

Window::~Window()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

int Window::Init()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "AnA", NULL, NULL);
    if (!window)
        return -1;

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, Window::FramebufferResizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwMakeContextCurrent(window);
    
    return 0;
}

#ifdef _async
std::thread loopThread;
void Window::StartLoop()
{
    loopThread = std::thread(&Window::mainLoop);
    loopThread.detach();
}

void Window::CloseLoop()
{
    glfwSetWindowShouldClose(window, GL_TRUE);
    if (loopThread.joinable())
    {
        loopThread.join();
    }
}
#else
void Window::StartLoop()
{
    mainLoop();
}
#endif

void Window::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto aWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    aWindow->FramebufferResized = true;
    aWindow->Width = width;
    aWindow->Height = height;
}