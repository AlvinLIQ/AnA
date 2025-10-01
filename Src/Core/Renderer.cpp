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
    VkCommandBuffer cmd = offscreenSecondaryCommandBuffers.Get();
    vkCmdExecuteCommands(commandBuffer, 
    1, 
    &cmd);
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

void Renderer::BeginRendering(CommandBuffer& commandBuffer)
{
    Device::ImageMemoryBarrier(commandBuffer, aSwapChain->swapChainImages[aSwapChain->CurrentImage], 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    Device::ImageMemoryBarrier(commandBuffer, aSwapChain->depthImages[aSwapChain->CurrentImage], 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // Color attachment
    VkRenderingAttachmentInfoKHR colorAttachmentInfo{};
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachmentInfo.imageView = aSwapChain->colorImageView;
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentInfo.clearValue = clearValues[0];
    colorAttachmentInfo.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachmentInfo.resolveImageView = aSwapChain->swapChainImageViews[aSwapChain->CurrentImage];
    colorAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingAttachmentInfoKHR depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.imageView = aSwapChain->depthImageViews[aSwapChain->CurrentImage];
    depthAttachment.clearValue = clearValues[1];

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = {};
    renderingInfo.renderArea.extent = aSwapChain->swapChainExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachmentInfo;
    renderingInfo.pDepthAttachment = &depthAttachment;
    aDevice->vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);
}

void Renderer::EndRendering(CommandBuffer& commandBuffer)
{
    aDevice->vkCmdEndRenderingKHR(commandBuffer);
    Device::ImageMemoryBarrier(commandBuffer, aSwapChain->swapChainImages[aSwapChain->CurrentImage], 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Device::ImageMemoryBarrier(commandBuffer, aSwapChain->depthImages[aSwapChain->CurrentImage], 
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void Renderer::EndRenderPass(CommandBuffer& commandBuffer)
{
    assert(isFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress!");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::BeginOffscreenRendering(CommandBuffer& commandBuffer)
{
    auto& framebuffer = aSwapChain->GetOffscreenFramebuffer();
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.albedo.image, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.position.image, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.normal.image, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.depth.image, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // --- Color Attachments ---
    std::array<VkRenderingAttachmentInfoKHR, 3> colorAttachments = {};

    for (int i = 0; i < 3; ++i) {
        colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachments[i].clearValue = clearValues[0];
    }
	colorAttachments[0].imageView = framebuffer.position.imageView;
	colorAttachments[1].imageView = framebuffer.normal.imageView;
	colorAttachments[2].imageView = framebuffer.albedo.imageView;

    VkRenderingAttachmentInfoKHR depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.imageView = framebuffer.depth.imageView;
    depthAttachment.clearValue = clearValues[1];

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = {};
    renderingInfo.renderArea.extent = aSwapChain->swapChainExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    renderingInfo.pColorAttachments = colorAttachments.data();
    renderingInfo.pDepthAttachment = &depthAttachment;
    aDevice->vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);
}

void Renderer::EndOffscreenRendering(CommandBuffer& commandBuffer)
{
    aDevice->vkCmdEndRenderingKHR(commandBuffer);
    auto& framebuffer = aSwapChain->GetOffscreenFramebuffer();
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.albedo.image, 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.position.image, 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.normal.image, 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Device::ImageMemoryBarrier(commandBuffer, framebuffer.depth.image, 
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
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