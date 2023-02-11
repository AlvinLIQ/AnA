#include "Headers/AnA_App.h" 
#include "Headers/AnA_Model.h"
#include "Headers/AnA_Object.h"
#include "Headers/AnA_Renderer.h"
#include "Headers/AnA_Camera.h"
#include "RenderSystem/Headers/AnA_RenderSystem.h"
#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <array>

using namespace AnA;

AnA_Model* _2DModel = nullptr;
AnA_Device* _aDevice;

AnA_App::AnA_App()
{

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
    aInstance = new AnA_Instance;
    aWindow->CreateWindowSurface(aInstance);
    _aDevice = aDevice = new AnA_Device(aInstance->GetInstance(), aWindow->GetSurface());
    loadObjects();
    aRenderer = new AnA_Renderer(aWindow, aDevice);
    aRenderSystem = new RenderSystems::AnA_RenderSystem(aDevice, aRenderer->GetSwapChain());
}

void AnA_App::Run()
{
    AnA_Camera camera;
    //aWindow->StartLoop();
    auto window = aWindow->GetGLFWwindow();
    //camera.SetViewDirection({}glm::vec3(0.5f, 0.f, 1.f));
    //camera.SetViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float aspect = aRenderer->GetAspect();
        camera.SetOrthographicProjection(-aspect, -1, aspect, 1, -1, 1);
        //camera.SetPerspectiveProjection(glm::radians(70.f), aspect, 0.1f, 10.f);

        if (auto commandBuffer = aRenderer->BeginFrame())
        {
            aRenderer->BeginSwapChainRenderPass(commandBuffer);
            aRenderSystem->RenderObjects(commandBuffer, objects, camera);
            aRenderer->EndSwapChainRenderPass(commandBuffer);
            aRenderer->EndFrame();
        }
    }

    vkDeviceWaitIdle(aDevice->GetLogicalDevice());
}

void AnA_App::Cleanup()
{
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
            {{-1.0f, 1.0f, 0.f}, {}},
            {{1.0f, -1.0f, 0.f}, {}},
            {{1.0f, 1.0f, 0.f}, {}}
        };
        _2DModel = new AnA_Model(_aDevice, vertices);
    }

    return _2DModel;
}

AnA_Model *CreateCubeModel(glm::vec3 offset)
{
    std::vector<AnA_Model::Vertex> vertices = 
    {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
    
        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
    
        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
    
        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
    
        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
    
        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
  };

    for (auto &v : vertices)
        v.position += offset;
    return new AnA_Model(_aDevice, vertices);
}

void AnA_App::loadObjects()
{
    auto cube = new AnA_Object;
    
    cube->Color = {0.1f, 0.2f, 0.3f};
    cube->Model = CreateCubeModel({});
    ItemProperties cubeProperties;
    cubeProperties.sType = ANA_MODEL;
    cubeProperties.transform.scale = {.5f, .5f, .5f};
    cubeProperties.transform.rotation = {};
    cubeProperties.transform.translation = {0.f, 0.f , .6f};

    cube->ItemsProperties.push_back(cubeProperties);
    objects.push_back(cube);
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
    itemProperties.sType = ANA_CURVED_RECTANGLE;
    itemProperties.color = {0.6f, 0.1f, 0.4f};
    itemProperties.transform.translation = {.1f, .1f, 0.f};
    shapes->ItemsProperties.push_back(itemProperties);
    itemProperties.sType = ANA_ELLIPSE;
    itemProperties.color = {0.1f, 0.6f, 0.4f};
    itemProperties.transform.translation = {.2f, .2f, 0.f};
    shapes->ItemsProperties.push_back(itemProperties);
    
    objects.push_back(shapes);
    */
}
