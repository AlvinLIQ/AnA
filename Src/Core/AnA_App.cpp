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
    aDevice = new AnA_Device(aInstance->GetInstance(), aWindow->GetSurface());
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

void AnA_App::loadObjects()
{
    std::vector<AnA_Model::Vertex> vertices
    {
        {{-1.0f, -1.0f}},
        {{1.0f, -1.0f}},
        {{-1.0f, 1.0f}},
        {{-1.0f, 1.0f}},
        {{1.0f, -1.0f}},
        {{1.0f, 1.0f}}
    };
    auto aModel = new AnA_Model(aDevice, vertices);

    auto rectangle = new AnA_Object;
    rectangle->Model = aModel;
    rectangle->Color = {0.8f, 0.8f, 0.8f};
    rectangle->Transform2D.scale = {1.f, 1.f};
    rectangle->Transform2D.rotation = 0.25f * glm::two_pi<float>();
    
    objects.push_back(rectangle);
}
