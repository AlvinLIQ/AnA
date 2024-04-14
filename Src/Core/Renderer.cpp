#include "Headers/Renderer.hpp"
#include "Headers/SwapChain.hpp"

#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

using namespace AnA;

Renderer::Renderer(Window& mWindow, Device& mDevice) : aWindow {mWindow}, aDevice {mDevice}
{
    aSwapChain = new SwapChain(aDevice, aWindow.GetSurface(), aWindow.GetGLFWwindow());
    inheritanceInfos[RENDER_PASS_TYPE_ONSCREEN].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfos[RENDER_PASS_TYPE_ONSCREEN].renderPass = aSwapChain->GetRenderPass();
    inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN].renderPass = aSwapChain->GetOffscreenRenderPass();
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

    delete offscreenSecondaryCommandBuffers;
}

VkCommandBuffer Renderer::BeginFrame()
{
    assert(!isFrameStarted && "Can't call BeginFrame while already in progress!");
    auto result = aSwapChain->AcquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        aSwapChain->RecreateSwapChain();
        needUpdate = true;
        return nullptr;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    needUpdate = false;
    isFrameStarted = true;

    auto commandBuffer = GetCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording buffer!");

    return commandBuffer;
}

void Renderer::RecordSecondaryCommandBuffer(RecordCallBack recordCallBack, RenderPassType renderPassType)
{
    auto swapChainExtent = aSwapChain->GetExtent();
    //aSwapChain->WaitForFences();
    auto& secondaryCommandBuffer = offscreenSecondaryCommandBuffers->Begin(&inheritanceInfos[renderPassType]);
    recordCallBack(secondaryCommandBuffer);

    offscreenSecondaryCommandBuffers->End();
}

void Renderer::ExcuteSecondaryCommandBuffer(VkCommandBuffer commandBuffer, RenderPassType renderPassType)
{
    vkCmdExecuteCommands(commandBuffer, 
    1, 
    &offscreenSecondaryCommandBuffers->Get());
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
        needUpdate = true;
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
    renderPassInfo.renderArea.offset = {};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& offset)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetSwapChainFramebuffers()[currentImageIndex];
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = {swapChainExtent.width - renderPassInfo.renderArea.offset.x, swapChainExtent.height - renderPassInfo.renderArea.offset.y};
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& offset, VkExtent2D& extent)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetSwapChainFramebuffers()[currentImageIndex];
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Renderer::EndRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress!");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame!");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::BeginOffscreenRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer& framebuffer)
{
    auto extent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassBegin;
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.pNext = NULL;
    renderPassBegin.renderPass = aSwapChain->GetOffscreenRenderPass();
    renderPassBegin.framebuffer = framebuffer;
    renderPassBegin.renderArea.offset.x = 0;
    renderPassBegin.renderArea.offset.y = 0;
    renderPassBegin.renderArea.extent = extent;
    renderPassBegin.clearValueCount = 1;
    renderPassBegin.pClearValues = &clearValues[RENDER_PASS_TYPE_OFFSCREEN];

    vkCmdBeginRenderPass(commandBuffer,
                        &renderPassBegin,
                        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

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

    offscreenSecondaryCommandBuffers = new CommandBuffer(aDevice, 
        MAX_FRAMES_IN_FLIGHT, 
        VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
}
