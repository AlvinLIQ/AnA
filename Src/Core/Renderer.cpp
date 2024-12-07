#include "Headers/Renderer.hpp"
#include "Headers/SwapChain.hpp"

#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

using namespace AnA;

Renderer::Renderer(Window& mWindow, Device* mDevice) : aWindow {mWindow}, aDevice{mDevice}, 
    commandBuffers{mDevice, MAX_FRAMES_IN_FLIGHT, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT},
    offscreenSecondaryCommandBuffers(mDevice, MAX_FRAMES_IN_FLIGHT, VK_COMMAND_BUFFER_LEVEL_SECONDARY, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
{
    aSwapChain = new SwapChain(aDevice, aWindow.GetSurface(), aWindow.GetGLFWwindow());
    inheritanceInfos[RENDER_PASS_TYPE_ONSCREEN].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfos[RENDER_PASS_TYPE_ONSCREEN].renderPass = aSwapChain->GetRenderPass();
    inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN].renderPass = aSwapChain->GetOffscreenRenderPass();
}

Renderer::~Renderer()
{
    delete aSwapChain;
}

VkCommandBuffer Renderer::BeginFrame()
{
    assert(!isFrameStarted && "Can't call BeginFrame while already in progress!");
    auto result = aSwapChain->AcquireNextImage();
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

    auto commandBuffer = commandBuffers.Begin();
    return commandBuffer;
}

void Renderer::RecordOffscreenSecondaryCommandBuffer(RecordCallBack recordCallBack)
{
    //aSwapChain->WaitForFences();
    auto& secondaryCommandBuffer = offscreenSecondaryCommandBuffers.Begin(&inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN]);
    recordCallBack(secondaryCommandBuffer);

    offscreenSecondaryCommandBuffers.End();
}

void Renderer::ExecuteOffscreenSecondaryCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdExecuteCommands(commandBuffer, 
    1, 
    &offscreenSecondaryCommandBuffers.Get());
}

void Renderer::EndFrame()
{
    assert(isFrameStarted && "Can't call EndFrame while frame is not in progress!");
    commandBuffers.End();
    auto commandBuffer = commandBuffers.Get();
    auto result = aSwapChain->SubmitCommandBuffers(&commandBuffer, 1); 

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
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetCurrentFramebuffer();
    renderPassInfo.renderArea.offset = {};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_EXT);
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetCurrentFramebuffer();
    renderPassInfo.renderArea.offset = {};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, contents);
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& offset)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetCurrentFramebuffer();
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = {swapChainExtent.width - renderPassInfo.renderArea.offset.x, swapChainExtent.height - renderPassInfo.renderArea.offset.y};
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& offset, VkExtent2D& extent)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetCurrentFramebuffer();
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& ltOffset, VkOffset2D& rbOffset)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    auto swapChainExtent = aSwapChain->GetExtent();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = aSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = aSwapChain->GetCurrentFramebuffer();
    renderPassInfo.renderArea.offset = ltOffset;
    renderPassInfo.renderArea.extent = {swapChainExtent.width - renderPassInfo.renderArea.offset.x - rbOffset.x, swapChainExtent.height - renderPassInfo.renderArea.offset.y - rbOffset.y};
    renderPassInfo.clearValueCount = numsof(clearValues);
    renderPassInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Renderer::EndRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress!");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::BeginOffscreenRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer& framebuffer, VkSubpassContents contents)
{
    VkExtent2D extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM};

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
                        contents);

}
