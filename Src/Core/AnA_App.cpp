#include "Headers/AnA_App.hpp" 
#include "Camera/Headers/AnA_CameraController.hpp"
#include "Headers/AnA_Model.hpp"
#include "Input/Headers/AnA_InputManager.hpp"
#include "RenderSystem/Headers/AnA_RenderSystem.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

using namespace AnA;

std::shared_ptr<AnA_Model> _2DModel;
AnA_Device *_aDevice;
AnA_App *_aApp;

#define UI_SIGNAL_EXIT 0
#define UI_SIGNAL_KEY 2
#define UI_SIGNAL_MOUSE 4
#define UI_SIGNAL_WAIT 1

bool _uiLoopShouldEnd = false;
short _uiSignal;
int _uiParam[2];

AnA_App::AnA_App()
{
    _aApp = this;
}

AnA_App::~AnA_App()
{
    Cleanup();
}

void AnA_App::Init()
{
    aWindow = new AnA_Window();
    if (aWindow->Init())
        throw std::runtime_error("Failed to init window!");
    aInputManager = new Input::AnA_InputManager(aWindow);

    //glfwSetKeyCallback(aWindow->GetGLFWwindow(), AnA_App::keyCallback);
    aInstance = new AnA_Instance;
    aWindow->CreateWindowSurface(aInstance);
    _aDevice = aDevice = new AnA_Device(aInstance->GetInstance(), aWindow->GetSurface());
    aRenderer = new AnA_Renderer(aWindow, aDevice);
    aRenderSystem = new RenderSystems::AnA_RenderSystem(aDevice, aRenderer->GetSwapChain());
}

void AnA_App::Run()
{
    Camera::AnA_CameraController cameraController{camera};
    cameraController.GetCameraKeyMapConfigs(aInputManager->GetKeyMapConfigs());
    cameraController.GetCameraCursorConfigs(aInputManager->GetCursorConfigs());
    //startUILoop(uiThread);
    auto window = aWindow->GetGLFWwindow();
    //camera.SetViewDirection({}, glm::vec3(0.5f, 0.f, 1.f));
    //camera.SetViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
    auto prevTime = std::chrono::high_resolution_clock::now();
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        auto curTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(curTime - prevTime).count();
        prevTime = curTime;
        camera.SetSpeedRatio(frameTime);
        if (aInputManager->CheckAndRunCallbacks())
            camera.UpdateViewMatrix();
        float aspect = aRenderer->GetAspect();
        //camera.SetOrthographicProjection(-aspect, -1, aspect, 1, -1, 1);
        camera.SetPerspectiveProjection(glm::radians(60.f), aspect, .1f, 100.f);
        
        if (auto commandBuffer = aRenderer->BeginFrame())
        {
            aRenderer->BeginSwapChainRenderPass(commandBuffer);
            aRenderSystem->RenderObjects(commandBuffer, objects, camera);
            aRenderer->EndSwapChainRenderPass(commandBuffer);
            aRenderer->EndFrame();
        }
    }
    //waitUILoop(uiThread);
    vkDeviceWaitIdle(aDevice->GetLogicalDevice());
}

void AnA_App::Cleanup()
{
    delete aInputManager;
    delete aRenderer;
    delete aRenderSystem;
    for (auto& object : objects)
    {
        delete object;
    }
    _2DModel.reset();
    delete aDevice;
    vkDestroySurfaceKHR(aInstance->GetInstance(), aWindow->GetSurface(), nullptr);
    delete aInstance;
    delete aWindow;
}

std::shared_ptr<AnA_Model> &AnA_App::Get2DModel()
{
    if (_2DModel == nullptr)
    {
        std::vector<AnA_Model::Vertex> vertices
        {
            {{-1.0f, -1.0f, 0.f}, {}},
            {{1.0f, -1.0f, 0.f}, {}},
            {{-1.0f, 1.0f, 0.f}, {}},
            {{1.0f, 1.0f, 0.f}, {}}
        };
        AnA_Model::ModelInfo modelInfo{vertices, 4, {0, 1, 2, 1, 2, 3}};
        _2DModel = std::make_shared<AnA_Model>(_aDevice, modelInfo);
    }

    return _2DModel;
}

void AnA_App::CreateModel(const AnA_Model::ModelInfo &modelInfo, std::shared_ptr<AnA_Model> &model)
{
    model = std::make_shared<AnA_Model>(_aDevice, modelInfo);
}

AnA_Model *CreateCubeModel()
{
    std::vector<AnA_Model::Vertex> vertices = 
    {
        // left (gray)
        {{-.5f, -.5f, -.5f},{.5f, .5f, .5f}},
        {{-.5f, -.5f, .5f}, {.5f, .5f, .5f}},
        {{-.5f, .5f, -.5f}, {.5f, .5f, .5f}},
        {{-.5f, -.5f, .5f}, {.5f, .5f, .5f}},
        {{-.5f, .5f, -.5f}, {.5f, .5f, .5f}},
        {{-.5f, .5f, .5f},  {.5f, .5f, .5f}},
        // right (red)
        {{.5f, -.5f, -.5f},{.9f, .2f, .2f}},
        {{.5f, -.5f, .5f}, {.9f, .2f, .2f}},
        {{.5f, .5f, -.5f}, {.9f, .2f, .2f}},
        {{.5f, -.5f, .5f}, {.9f, .2f, .2f}},
        {{.5f, .5f, -.5f}, {.9f, .2f, .2f}},
        {{.5f, .5f, .5f},  {.9f, .2f, .2f}},
        // top (yellow)
        {{-.5f, -.5f, -.5f},{.9f, .9f, .2f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .2f}},
        {{.5f, -.5f, -.5f}, {.9f, .9f, .2f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .2f}},
        {{.5f, -.5f, -.5f}, {.9f, .9f, .2f}},
        {{.5f, -.5f, .5f},  {.9f, .9f, .2f}},
        // bottom (white)
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f},  {.9f, .9f, .9f}},
        {{.5f,  .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f},  {.9f, .9f, .9f}},
        {{.5f,  .5f, -.5f}, {.9f, .9f, .9f}},
        {{.5f,  .5f, .5f},  {.9f, .9f, .9f}},
        // back (green)
        {{-.5f, -.5f, .5f}, {.2f, .9f, .2f}},
        {{-.5f, .5f, .5f},  {.2f, .9f, .2f}},
        {{.5f,  -.5f, .5f}, {.2f, .9f, .2f}},
        {{-.5f, .5f, .5f},  {.2f, .9f, .2f}},
        {{.5f,  -.5f, .5f}, {.2f, .9f, .2f}},
        {{.5f,  .5f, .5f},  {.2f, .9f, .2f}},
        // front (blue)
        {{-.5f, -.5f, -.5f}, {.2f, .2f, .9f}},
        {{-.5f, .5f, -.5f},  {.2f, .2f, .9f}},
        {{.5f,  -.5f, -.5f}, {.2f, .2f, .9f}},
        {{-.5f, .5f, -.5f},  {.2f, .2f, .9f}},
        {{.5f,  -.5f, -.5f}, {.2f, .2f, .9f}},
        {{.5f,  .5f, -.5f},  {.2f, .2f, .9f}},
    };

    return new AnA_Model(_aDevice, {vertices, 4, {0, 1, 2, 3, 4, 5}});
}

void AnA_App::startUILoop(std::thread &loopThread)
{
    _uiSignal = UI_SIGNAL_WAIT;
    loopThread = std::thread(AnA_App::uiLoop);
    loopThread.detach();
}

void AnA_App::waitUILoop(std::thread &loopThread)
{
    _uiLoopShouldEnd = true;
    if (loopThread.joinable())
        loopThread.join();
}

void AnA_App::uiLoop()
{
    while (!_uiLoopShouldEnd)
    {
        switch (_uiSignal)
        {
        case UI_SIGNAL_KEY:
            {/*
            auto &camera = _aApp->GetCamera();
            auto &key = _uiParam[0];
            if (key == GLFW_KEY_W)
                camera.MoveForward();
            if (key == GLFW_KEY_S)
                camera.MoveBack();
            if (key == GLFW_KEY_A)
                camera.MoveLeft();
            if (key == GLFW_KEY_D)
                camera.MoveRight();
            if (key == GLFW_KEY_SPACE)
                camera.MoveUp();
            if (key == GLFW_KEY_C)
                camera.MoveDown();*/
            }
            _uiSignal = UI_SIGNAL_WAIT;
            break;
        case UI_SIGNAL_EXIT:
            return;
        }
    }
}

void AnA_App::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    /*
    _uiParam[0] = key;
    _uiParam[1] = action;
    _uiSignal = UI_SIGNAL_KEY;
    */
    //glfwGetKey
}