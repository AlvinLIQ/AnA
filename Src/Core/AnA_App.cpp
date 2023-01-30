#include "Headers/AnA_App.h" 
#include "Headers/AnA_Model.h"
#include "Headers/AnA_Object.h"
#include "Headers/AnA_Renderer.h"
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

struct SimplePushConstantData
{
    glm::mat2 transform {1.f};
    glm::uint32_t sType;
    alignas(8) glm::vec2 offset;
    glm::vec2 size;
    glm::vec2 resolution;
    alignas(16) glm::vec3 color;
};

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
    createPipelineLayout();
    aPipeline = new AnA_Pipeline(aDevice, aRenderer->GetSwapChain(), pipelineLayout);
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
            renderObjects(commandBuffer);
            aRenderer->EndSwapChainRenderPass(commandBuffer);
            aRenderer->EndFrame();
        }
    }

    vkDeviceWaitIdle(aDevice->GetLogicalDevice());
}

void AnA_App::Cleanup()
{
    delete aRenderer;
    delete aPipeline;
    //Cleanup pipelines before pipelineLayout
    vkDestroyPipelineLayout(aDevice->GetLogicalDevice(), pipelineLayout, nullptr);
    for (auto& object : objects)
    {
        delete object;
    }
    delete aDevice;
    vkDestroySurfaceKHR(aInstance->GetInstance(), aWindow->GetSurface(), nullptr);
    delete aInstance;
    delete aWindow;
}

void AnA_App::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(aDevice->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

void AnA_App::renderObjects(VkCommandBuffer commandBuffer)
{
    aPipeline->Bind(commandBuffer);
    for (auto& object : objects)
    {
        object->Model->Bind(commandBuffer);

        SimplePushConstantData push{};
        push.offset = object->Transform2D.translation;
        push.color = object->Color;
        push.transform = object->Transform2D.mat2();

/*        int width, height;
        glfwGetFramebufferSize(aWindow->GetGLFWwindow(), &width, &height);
        push.resolution = {(float)width, (float)height};*/

        auto extent = aRenderer->GetSwapChainExtent();
        push.resolution = {extent.width, extent.height};

        vkCmdPushConstants(commandBuffer, 
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);
        object->Model->Draw(commandBuffer);
    }
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
