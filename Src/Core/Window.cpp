#include "Headers/Window.hpp"

using namespace AnA;

Window::Window(const char* title)
{
    if (init(title))
        throw std::runtime_error("Failed to init window!");
}

Window::~Window()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Window::PollEvents()
{
    if (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            exitSignal = true;
            return;
        }
        switch(event.type)
        {
            case SDL_EVENT_MOUSE_MOTION:
                cursorPos.x = event.motion.x;
                cursorPos.y = event.motion.y;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                [[fallthrough]];
            case SDL_EVENT_MOUSE_BUTTON_UP:
                leftButton = event.button.down;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (KeyCallback)
                    KeyCallback(event.key.scancode, ANA_PRESS);
                break;
            case SDL_EVENT_KEY_UP:
                if (KeyCallback)
                    KeyCallback(event.key.scancode, ANA_RELEASE);
                break;
            case SDL_EVENT_FINGER_UP:
                leftButton = 0;
                break;
            case SDL_EVENT_FINGER_MOTION:
            {
                auto fingers = SDL_GetTouchFingers(event.tfinger.touchID, &fingerCount);
                if (fingerCount == 2)
                {
                    auto dPos = glm::vec2(fingers[0]->x - fingers[1]->x,
                        fingers[0]->y - fingers[1]->y);
                    float fingerDistance = glm::length(dPos);
                    prevFingerDistance = fingerDistance;
                }
            }
                break;
            case SDL_EVENT_MOUSE_WHEEL:
            {
                if (ScrollCallback)
                    ScrollCallback(event.wheel.x, event.wheel.y);
            }
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                Width = event.window.data1;
                Height = event.window.data2;
                FramebufferResized = true;
                break;
            default:
                break;
        }
    }
}

int Window::init(const char* title)
{
    SDL_SetAppMetadata(title, "1.0", "com.engine.ana");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!(window = SDL_CreateWindow(title, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE)))
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

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
    while (exitSignal)
    {
        PollEvents();
    }
}

/*
void Window::FramebufferResizeCallback(GLFWwindow* window, int , int )
{
    auto aWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    aWindow->FramebufferResized = true;
    glfwGetFramebufferSize(window, &aWindow->Width, &aWindow->Height);
}
*/
