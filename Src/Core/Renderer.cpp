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
    createTimestampQueryPool();
    inheritanceInfos[RENDER_PASS_TYPE_ONSCREEN].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfos[RENDER_PASS_TYPE_ONSCREEN].renderPass = aSwapChain->GetRenderPass();
    inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN].renderPass = aSwapChain->GetOffscreenRenderPass();
}

Renderer::~Renderer()
{
    vkDestroyQueryPool(aDevice->GetLogicalDevice(), timestampQueryPool, nullptr);
    delete aSwapChain;
}

CommandBuffer* Renderer::BeginFrame()
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

    commandBuffers.Begin();
    firstQuery = aSwapChain->CurrentFrame * 2;
    vkCmdResetQueryPool(commandBuffers, timestampQueryPool, 
        firstQuery, queriesPerFrame);
    if (timestamps[firstQuery + 1])
        vkCmdWriteTimestamp(commandBuffers, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
            timestampQueryPool, firstQuery);

    return &commandBuffers;
}

void Renderer::RecordOffscreenSecondaryCommandBuffer(RecordCallBack recordCallBack)
{
    //aSwapChain->WaitForFences();
    offscreenSecondaryCommandBuffers.Begin(&inheritanceInfos[RENDER_PASS_TYPE_OFFSCREEN]);
    recordCallBack(offscreenSecondaryCommandBuffers);

    offscreenSecondaryCommandBuffers.End();
}

void Renderer::ExecuteOffscreenSecondaryCommandBuffer(CommandBuffer& commandBuffer)
{
    vkCmdExecuteCommands(commandBuffer, 
    1, 
    &offscreenSecondaryCommandBuffers.Get());
}

void Renderer::EndFrame()
{
    assert(isFrameStarted && "Can't call EndFrame while frame is not in progress!");
    if (timestamps[firstQuery + 2])
        vkCmdWriteTimestamp(commandBuffers, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
            timestampQueryPool, firstQuery + 1);
    commandBuffers.End();
    auto commandBuffer = commandBuffers.Get();
    auto result = aSwapChain->SubmitCommandBuffers(&commandBuffer, 1); 

    vkGetQueryPoolResults(
        aDevice->GetLogicalDevice(),
        timestampQueryPool,
        firstQuery,
        queriesPerFrame,
        sizeof(timestamps),
        timestamps,
        sizeof(uint64_t) * queriesPerFrame,
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);

    if (timestamps[1] && timestamps[3])
    {
        const float timestampPeriod = aDevice->GetPhysicalDeviceProperties().limits.timestampPeriod;
        gpuTime = float(timestamps[2] - timestamps[0]) * 
                timestampPeriod / 1000000.0f;
    }
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

void Renderer::BeginSwapChainRenderPass(CommandBuffer& commandBuffer)
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

void Renderer::BeginSwapChainRenderPass(CommandBuffer& commandBuffer, VkSubpassContents contents)
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

void Renderer::BeginSwapChainRenderPass(CommandBuffer& commandBuffer, VkOffset2D& offset)
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

void Renderer::BeginSwapChainRenderPass(CommandBuffer& commandBuffer, VkOffset2D& offset, VkExtent2D& extent)
{
    assert(isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress!");
    //auto swapChainExtent = aSwapChain->GetExtent();

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

void Renderer::BeginSwapChainRenderPass(CommandBuffer& commandBuffer, VkOffset2D& ltOffset, VkOffset2D& rbOffset)
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

void Renderer::EndRenderPass(CommandBuffer& commandBuffer)
{
    assert(isFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress!");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::BeginOffscreenRenderPass(CommandBuffer& commandBuffer, VkFramebuffer& framebuffer, VkSubpassContents contents)
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

void Renderer::createTimestampQueryPool()
{
    VkQueryPoolCreateInfo queryPoolInfo = {};
    queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolInfo.queryCount = queriesPerFrame * MAX_FRAMES_IN_FLIGHT;
    vkCreateQueryPool(aDevice->GetLogicalDevice(), &queryPoolInfo, 
        nullptr, &timestampQueryPool);
}