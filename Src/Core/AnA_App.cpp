#include "Headers/AnA_App.hpp" 
#include "Camera/Headers/AnA_CameraController.hpp"
#include "Input/Headers/AnA_InputManager.hpp"
#include "RenderSystem/Headers/AnA_RenderSystem.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <vulkan/vulkan_core.h>

using namespace AnA;

AnA_Model *_2DModel = nullptr;
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
    loadObjects();
    aRenderer = new AnA_Renderer(aWindow, aDevice);
    aRenderSystem = new RenderSystems::AnA_RenderSystem(aDevice, aRenderer->GetSwapChain());
}

void AnA_App::Run()
{
    Camera::AnA_CameraController cameraController{camera};
    cameraController.GetCameraKeyMapConfigs(aInputManager->GetKeyMapConfigs());
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

        aInputManager->CheckAndRunCallbacks();
        float aspect = aRenderer->GetAspect();
        //camera.SetOrthographicProjection(-aspect, -1, aspect, 1, -1, 1);
        camera.SetPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);
        
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
    delete aDevice;
    vkDestroySurfaceKHR(aInstance->GetInstance(), aWindow->GetSurface(), nullptr);
    delete aInstance;
    delete aWindow;
}

AnA_Model *AnA_App::Get2DModel()
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
        _2DModel = new AnA_Model(_aDevice, {vertices, 4, {0, 1, 2, 1, 2, 3}});
    }

    return _2DModel;
}

void AnA_App::CreateModel(const AnA_Model::ModelInfo &modelInfo, AnA_Model** pModel)
{
    if (pModel == nullptr)
        throw std::runtime_error("pModel is nullptr!");
    *pModel = new AnA_Model(_aDevice, modelInfo);
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

void AnA_App::loadObjects()
{
    /*
    auto cube = new AnA_Object;
    
    cube->Color = {0.1f, 0.2f, 0.3f};
    cube->Model = CreateCubeModel();
    ItemProperties cubeProperties;
    cubeProperties.sType = ANA_MODEL;
    cubeProperties.transform.scale = {.4f, .4f, .4f};
    cubeProperties.transform.rotation = glm::vec3(0.04f * glm::two_pi<float>(), 0.f, 0.f);
    cubeProperties.transform.translation = {0.f, 0.f , 1.5f};

    cube->ItemsProperties.push_back(std::move(cubeProperties));
    objects.push_back(std::move(cube));*/
    /*
    auto shapes = new AnA_Object;
    shapes->Model = Get2DModel();
    shapes->Color = {0.8f, 0.8f, 0.8f};

    ItemProperties itemProperties;
    itemProperties.sType = ANA_RECTANGLE;
    itemProperties.color = {0.1f, 0.4f, 0.6f};
    itemProperties.transform.scale = {.2f, .2f, 1.f};
    itemProperties.transform.rotation = {};//0.25f * glm::two_pi<float>();
    itemProperties.transform.translation = {.0f, .0f , 0.f};
    shapes->ItemsProperties.push_back(itemProperties);
    itemProperties.sType = ANA_ELLIPSE;
    itemProperties.color = {0.1f, 0.6f, 0.4f};
    itemProperties.transform.translation = {.2f, .2f, 0.1f};
    shapes->ItemsProperties.push_back(itemProperties);
    itemProperties.sType = ANA_CURVED_RECTANGLE;
    itemProperties.color = {0.6f, 0.1f, 0.4f};
    itemProperties.transform.translation = {.1f, .1f, 0.0f};
    shapes->ItemsProperties.push_back(itemProperties);
    
    objects.push_back(shapes);
    */
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