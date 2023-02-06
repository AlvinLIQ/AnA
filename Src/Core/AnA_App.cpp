#include "Headers/AnA_App.h" 
#include "Headers/AnA_Model.h"
#include "Headers/AnA_Object.h"
#include "Headers/AnA_Renderer.h"
#include "RenderSystem/Headers/AnA_RenderSystem.h"
#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
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
    //aWindow->StartLoop();
    auto window = aWindow->GetGLFWwindow();
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (auto commandBuffer = aRenderer->BeginFrame())
        {
            aRenderer->BeginSwapChainRenderPass(commandBuffer);
            aRenderSystem->RenderObjects(commandBuffer, objects);
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
            {{-1.0f, -1.0f, 0.f}},
            {{1.0f, -1.0f, 0.f}},
            {{-1.0f, 1.0f, 0.f}},
            {{-1.0f, 1.0f, 0.f}},
            {{1.0f, -1.0f, 0.f}},
            {{1.0f, 1.0f, 0.f}}
        };
        _2DModel = new AnA_Model(_aDevice, vertices);
    }

    return _2DModel;
}

void AnA_App::loadObjects()
{
    auto shapes = new AnA_Object;
    shapes->Model = Get2DModel();
    shapes->Color = {0.8f, 0.8f, 0.8f};

    ItemProperties itemProperties;
    itemProperties.sType = ANA_RECTANGLE;
    itemProperties.color = {0.1f, 0.4f, 0.6f};
    itemProperties.transform.scale = {.2f, .4f, 1.f};
    itemProperties.transform.rotation = {};//0.25f * glm::two_pi<float>();
    itemProperties.transform.translation = {.0f, .0f , .0f};
    shapes->ItemsProperties.push_back(itemProperties);
    itemProperties.sType = ANA_CURVED_RECTANGLE;
    itemProperties.color = {0.6f, 0.1f, 0.4f};
    itemProperties.transform.translation = {.1f, .1f, .0f};
    shapes->ItemsProperties.push_back(itemProperties);
    itemProperties.sType = ANA_ELLIPSE;
    itemProperties.color = {0.1f, 0.6f, 0.4f};
    itemProperties.transform.translation = {.2f, .2f, .0f};
    shapes->ItemsProperties.push_back(itemProperties);
    
    objects.push_back(shapes);
}
