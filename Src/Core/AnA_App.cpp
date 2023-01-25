#include "Headers/AnA_App.h" 
#include "Headers/AnA_Model.h"
#include "Headers/AnA_Object.h"
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <array>

using namespace AnA;

struct SimplePushConstantData
{
    glm::mat2 transform {1.f};
    glm::vec2 offset;
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
    aWindow = new AnA_Window;
    if (aWindow->Init(this, AnA_App::FramebufferResizeCallback))
    {
        throw std::runtime_error("Couldn't create GLFW window!");
    }
    aInstance = new AnA_Instance;
    aSurface = new AnA_Surface(aInstance->GetInstance(), aWindow->GetGLFWwindow());
    aDevice = new AnA_Device(aInstance->GetInstance(), aSurface->GetSurface());
    aSwapChain = new AnA_SwapChain(aDevice, aSurface->GetSurface(), aWindow->GetGLFWwindow());
    loadObjects();
    createPipelineLayout();
    aPipeline = new AnA_Pipeline(aDevice, aSwapChain, pipelineLayout);
    createCommandPool();
    createCommandBuffer();
}

void AnA_App::Run()
{
    //aWindow->StartLoop();
    auto window = aWindow->GetGLFWwindow();
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(aDevice->GetLogicalDevice());
}

void AnA_App::Cleanup()
{
    vkDestroyCommandPool(aDevice->GetLogicalDevice(), commandPool, nullptr);
    delete aPipeline;
    //Cleanup pipelines before pipelineLayout
    vkDestroyPipelineLayout(aDevice->GetLogicalDevice(), pipelineLayout, nullptr);
    for (auto& object : objects)
    {
        delete object;
    }
    delete aSwapChain;
    delete aDevice;
    delete aSurface;
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

void AnA_App::createCommandPool()
{
    AnA_Device::QueueFamilyIndices queueFamilyIndices = aDevice->GetQueueFamiliesForCurrent();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(aDevice->GetLogicalDevice(), &poolInfo, 
    nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");
}

void AnA_App::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording buffer!");
    
    auto swapChainExtent = aSwapChain->GetExtend();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetSwapChainFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.extent = swapChainExtent;
    scissor.offset = {0, 0};

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    renderObjects(commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
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
        push.color = {0.6f, 0.6f, 0.6f};
        push.transform = object->Transform2D.mat2();

        vkCmdPushConstants(commandBuffer, 
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);
        object->Model->Draw(commandBuffer);
    }
}

void AnA_App::createCommandBuffer()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    if (vkAllocateCommandBuffers(aDevice->GetLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
        throw std::runtime_error("Failed to allocate command buffers!");
}

void AnA_App::drawFrame()
{
    uint32_t imageIndex;
    auto result = aSwapChain->AcquireNextImage(&imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        aSwapChain->RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    vkResetCommandBuffer(commandBuffers[aSwapChain->CurrentFrame], 0);
    recordCommandBuffer(commandBuffers[aSwapChain->CurrentFrame], imageIndex);
    result = aSwapChain->SubmitCommandBuffers(&commandBuffers[aSwapChain->CurrentFrame],  &imageIndex); 

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FramebufferResized)
    {
        FramebufferResized = false;
        aSwapChain->RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
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

void AnA_App::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<AnA_App*>(glfwGetWindowUserPointer(window));
    app->FramebufferResized = true;
}