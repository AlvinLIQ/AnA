#include "Headers/Renderer.hpp"
#include "Headers/SwapChain.hpp"

#include <cassert>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>

using namespace AnA;

Renderer::Renderer(Window& mWindow, Device& mDevice) : aWindow {mWindow}, aDevice {mDevice}
{
    aSwapChain = new SwapChain(aDevice, aWindow.GetSurface(), aWindow.GetGLFWwindow());
    createCommandBuffers();
}

Renderer::~Renderer()
{
    delete aSwapChain;
    vkFreeCommandBuffers(aDevice.GetLogicalDevice(), 
        aDevice.GetCommandPool(), 
        static_cast<uint32_t>(commandBuffers.size()), 
        commandBuffers.data());

    commandBuffers.clear();

    vkFreeCommandBuffers(aDevice.GetLogicalDevice(), 
        aDevice.GetCommandPool(), 
        static_cast<uint32_t>(secondaryCommandBuffers.size()), 
        secondaryCommandBuffers.data());

    secondaryCommandBuffers.clear();
}

VkCommandBuffer Renderer::BeginFrame()
{
    assert(!isFrameStarted && "Can't call BeginFrame while already in progress!");
    auto result = aSwapChain->AcquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        aSwapChain->RecreateSwapChain();
        return nullptr;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    isFrameStarted = true;
    
    auto commandBuffer = GetCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording buffer!");

    return commandBuffer;
}

void Renderer::RecordSecondaryCommandBuffers(void(*recordCallBack)(VkCommandBuffer commandBuffer))
{
    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = aSwapChain->GetRenderPass();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    auto swapChainExtent = aSwapChain->GetExtent();
    for (auto& secondaryCommandBuffer : secondaryCommandBuffers)
    {
        if (vkBeginCommandBuffer(secondaryCommandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording secondary buffer!");

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(secondaryCommandBuffer, 0, 1, &viewport);
        
        VkRect2D scissor{};
        scissor.extent = swapChainExtent;
        scissor.offset = {0, 0};

        vkCmdSetScissor(secondaryCommandBuffer, 0, 1, &scissor);
        recordCallBack(secondaryCommandBuffer);

        vkEndCommandBuffer(secondaryCommandBuffer);
    }

}

void Renderer::ExcuteSecondaryCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdExecuteCommands(commandBuffer, 1, &secondaryCommandBuffers[currentFrameIndex]);
}

void Renderer::EndFrame()
{
    assert(isFrameStarted && "Can't call EndFrame while frame is not in progress!");
    auto commandBuffer = GetCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
    auto result = aSwapChain->SubmitCommandBuffers(&commandBuffer, 1,  &currentImageIndex); 

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || aWindow.FramebufferResized)
    {
        aWindow.FramebufferResized = false;
        aSwapChain->RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetSwapChainFramebuffers()[currentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress!");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame!");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::createCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = aDevice.GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    if (vkAllocateCommandBuffers(aDevice.GetLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
        throw std::runtime_error("Failed to allocate command buffers!");

    secondaryCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    if (vkAllocateCommandBuffers(aDevice.GetLogicalDevice(), &allocInfo, secondaryCommandBuffers.data()) != VK_SUCCESS) 
        throw std::runtime_error("Failed to allocate secondary command buffers!");
}
