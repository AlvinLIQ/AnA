#include "Headers/SwapChain.hpp"
#include "Resources/Headers/CommandBuffer.hpp"
#include "vulkan/vulkan_core.h"
#include <algorithm>
#include <limits>

using namespace AnA;
SwapChain* _swapChain;
SwapChain::SwapChain(Device* mDevice,
                             VkSurfaceKHR &mSurface, Window* mWindow) : aDevice{mDevice}, surface{mSurface}, window{mWindow}
{
    msaaSamplers = aDevice->GetMaxUsableSampleCount();
    createSwapChain();
    createImageViews();
    createColorResources();
    createDepthResources();
    createOffscreenSampler();
    createOffscreenFramebuffer();
    createSyncObjects();

    _swapChain = this;
}
SwapChain::~SwapChain()
{
    auto device = aDevice->GetLogicalDevice();
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
        offScreenFrameBuffers[i].cleanup(aDevice);
    }
    vkDestroySampler(device, colorSampler, nullptr);
    cleanupSwapChain();
}

void SwapChain::WaitForFence()
{
    vkWaitForFences(aDevice->GetLogicalDevice(), 1, &inFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);
}

void SwapChain::ResetFence()
{
    vkResetFences(aDevice->GetLogicalDevice(), 1, &inFlightFences[CurrentFrame]);
}

VkResult SwapChain::AcquireNextImage()
{
    auto result = vkAcquireNextImageKHR(aDevice->GetLogicalDevice(), swapChain, UINT64_MAX,
                                 imageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE,
                                 &CurrentImage);

    return result;
}

VkResult SwapChain::SubmitCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkCommandBufferSubmitInfo commandBufferSubmitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = VK_NULL_HANDLE,
        .commandBuffer = commandBuffer,
        .deviceMask = 0
    };
    std::unique_lock<std::mutex> lock(queueMutex);
    commandBuffersSubmitQueue.emplace_back(commandBufferSubmitInfo);
    return VK_SUCCESS;
}

VkResult SwapChain::SubmitCommandBufferQueue()
{
    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

    VkSemaphoreSubmitInfoKHR waitSemaphoreSubmitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = VK_NULL_HANDLE,
        .semaphore = imageAvailableSemaphores[CurrentFrame],
        .value = 1, // replaces VkTimelineSemaphoreSubmitInfo
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        .deviceIndex = 0, // replaces VkDeviceGroupSubmitInfo
    };
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;
    submitInfo.waitSemaphoreInfoCount = 1;

    VkSemaphoreSubmitInfoKHR signalSemaphoreSubmitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = VK_NULL_HANDLE,
        .semaphore = renderFinishedSemaphores[CurrentImage],
        .value = 2, // replaces VkTimelineSemaphoreSubmitInfo
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0, // replaces VkDeviceGroupSubmitInfo
    };

    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;
    submitInfo.signalSemaphoreInfoCount = 1;

    VkResult result;
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        submitInfo.pCommandBufferInfos = commandBuffersSubmitQueue.data();
        submitInfo.commandBufferInfoCount = uint32_t(commandBuffersSubmitQueue.size());
        if ((result = vkQueueSubmit2(aDevice->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[CurrentFrame])) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        commandBuffersSubmitQueue.clear();
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &signalSemaphoreSubmitInfo.semaphore;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &CurrentImage;

    vkQueuePresentKHR(aDevice->GetPresentQueue(), &presentInfo);

    CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return result;
}

SwapChain* SwapChain::GetCurrent()
{
    return _swapChain;
}

VkExtent2D& SwapChain::GetExtent()
{
    return swapChainExtent;
}

VkFormat SwapChain::GetFormat()
{
    return swapChainImageFormat;
}

VkFormat SwapChain::GetDepthFormat()
{
    return swapChainDepthFormat;
}

uint32_t SwapChain::GetImageCount()
{
    return static_cast<uint32_t>(swapChainImageViews.size());
}

void SwapChain::SetViewport(CommandBuffer& commandBuffer)
{
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    commandBuffer.Offset = scissor.offset;
    commandBuffer.Extent = scissor.extent;

}

void SwapChain::SetViewport(CommandBuffer& commandBuffer, VkOffset2D& offset)
{
    VkViewport _viewport = {(float)offset.x, (float)offset.y, static_cast<float>(swapChainExtent.width - offset.x),
        static_cast<float>(swapChainExtent.height - offset.y), 0.0f, 1.0f};
    VkRect2D _scissor = {offset, {swapChainExtent.width - offset.x, swapChainExtent.height - offset.y}};
    vkCmdSetViewport(commandBuffer, 0, 1, &_viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &_scissor);
    commandBuffer.Offset = _scissor.offset;
    commandBuffer.Extent = _scissor.extent;
}

void SwapChain::SetViewport(CommandBuffer& commandBuffer, VkExtent2D& extent)
{
    VkViewport _viewport = {0.0f, 0.0f, static_cast<float>(extent.width),
        static_cast<float>(extent.height), 0.0f, 1.0f};
    VkRect2D _scissor = {{0, 0}, extent};
    vkCmdSetViewport(commandBuffer, 0, 1, &_viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &_scissor);
    commandBuffer.Offset = _scissor.offset;
    commandBuffer.Extent = _scissor.extent;
}

void SwapChain::SetViewport(CommandBuffer& commandBuffer, VkOffset2D& offset, VkExtent2D& extent)
{
    VkViewport _viewport = {(float)offset.x, (float)offset.y, static_cast<float>(extent.width),
        static_cast<float>(extent.height), 0.0f, 1.0f};
    VkRect2D _scissor = {offset, {extent.width, extent.height}};
    vkCmdSetViewport(commandBuffer, 0, 1, &_viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &_scissor);
    commandBuffer.Offset = _scissor.offset;
    commandBuffer.Extent = _scissor.extent;
}

void SwapChain::RecreateSwapChain()
{
    int width, height;
    do
    {
        window->PollEvents();
        width = window->Width;
        height = window->Height;
        //glfwWaitEvents();
    } while (width == 0 || height == 0);

    vkDeviceWaitIdle(aDevice->GetLogicalDevice());
    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createColorResources();
    createDepthResources();
    createOffscreenFramebuffer();
}

Device* SwapChain::GetDevice()
{
    return aDevice;
}

VkSemaphore& SwapChain::GetCurrentSemaphore()
{
    return renderFinishedSemaphores[CurrentImage];
}

VkFence& SwapChain::GetCurrentFence()
{
    return inFlightFences[CurrentFrame];
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return availablePresentModes.front();
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {static_cast<uint32_t>(window->Width), static_cast<uint32_t>(window->Height)};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

void SwapChain::createSwapChain()
{
    //glfwGetWindowContentScale(window, &ScaleX, &ScaleY);
    window->GetScale(&ScaleX, &ScaleY);

    Device::SwapChainSupportDetails swapChainSupport = aDevice->QuerySwapChainSupport(aDevice->GetPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Device::QueueFamilyIndices indices = aDevice->FindQueueFamilies(aDevice->GetPhysicalDevice());
    uint32_t queueFamilyIndices[] = {indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsAndComputeFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(aDevice->GetLogicalDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain!");

    vkGetSwapchainImagesKHR(aDevice->GetLogicalDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(aDevice->GetLogicalDevice(), swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.extent = swapChainExtent;
    scissor.offset = {0, 0};
}

void SwapChain::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = aDevice->CreateImageView(swapChainImages[i], swapChainImageFormat);
    }
}

VkFormat SwapChain::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(aDevice->GetPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat SwapChain::findDepthFormat()
{
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void SwapChain::createColorResources()
{
    VkFormat colorFormat = swapChainImageFormat;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = colorFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.samples = msaaSamplers;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    aDevice->CreateImage(&imageInfo, &colorImage, colorImageAllocation);
    /*
    if (aDevice->HostImageCopySupport())
    {
        VkHostImageLayoutTransitionInfo transitionInfo{};
        transitionInfo.sType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO;
        transitionInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        transitionInfo.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        transitionInfo.image = colorImage;
        transitionInfo.subresourceRange =
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        };
        vkTransitionImageLayout(aDevice->GetLogicalDevice(), 1, &transitionInfo);
    }
    else
    {
        auto imageBarrier = Device::ImageMemoryBarrier2(
            colorImage,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_ACCESS_2_NONE,
            VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_NONE,
            VK_PIPELINE_STAGE_2_NONE,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        auto commandbuffer = aDevice->BeginSingleTimeCommands();
        Device::PipelineBarrier2(commandbuffer, VK_DEPENDENCY_BY_REGION_BIT,
            &imageBarrier,
            1,
            VK_NULL_HANDLE,
            0);
        aDevice->EndSingleTimeCommands(commandbuffer);
    }*/
    colorImageView = aDevice->CreateImageView(colorImage, colorFormat);
}

void SwapChain::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();
    swapChainDepthFormat = depthFormat;
    VkExtent2D swapChainExtent = GetExtent();

    depthImages.resize(swapChainImages.size());
    depthImageAllocations.resize(depthImages.size());
    depthImageViews.resize(depthImages.size());

    for (size_t i = 0; i < depthImages.size(); i++)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = msaaSamplers;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        aDevice->CreateImage(&imageInfo, &depthImages[i], depthImageAllocations[i]);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = depthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(aDevice->GetLogicalDevice(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}

void SwapChain::createOffscreenFramebuffer()
{
    for (auto& offScreenFrameBuffer: offScreenFrameBuffers)
    {
        offScreenFrameBuffer.width = swapChainExtent.width;
        offScreenFrameBuffer.height = swapChainExtent.height;

        offScreenFrameBuffer.position.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        offScreenFrameBuffer.position.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        offScreenFrameBuffer.position.extent = {offScreenFrameBuffer.width, offScreenFrameBuffer.height, 1};
        offScreenFrameBuffer.position.create(aDevice, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        offScreenFrameBuffer.normal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        offScreenFrameBuffer.normal.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        offScreenFrameBuffer.normal.extent = offScreenFrameBuffer.position.extent;
        offScreenFrameBuffer.normal.create(aDevice, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        offScreenFrameBuffer.albedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        offScreenFrameBuffer.albedo.format = VK_FORMAT_R8G8B8A8_UNORM;
        offScreenFrameBuffer.albedo.extent = offScreenFrameBuffer.position.extent;
        offScreenFrameBuffer.albedo.create(aDevice, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        offScreenFrameBuffer.depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        offScreenFrameBuffer.depth.format = swapChainDepthFormat;
        offScreenFrameBuffer.depth.extent = offScreenFrameBuffer.position.extent;
        offScreenFrameBuffer.depth.create(aDevice,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1,
            { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });
    }
}

void SwapChain::createOffscreenSampler()
{
    aDevice->CreateSampler(&colorSampler, VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
}

void SwapChain::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    auto device = aDevice->GetLogicalDevice();
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(swapChainImages.size());
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0 ; i < renderFinishedSemaphores.size(); i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create semaphores!");
    }
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create semaphores or fences!");
    }
}

void SwapChain::cleanupSwapChain()
{
    auto device = aDevice->GetLogicalDevice();
    for (auto& offScreenFrameBuffer : offScreenFrameBuffers)
        offScreenFrameBuffer.cleanupImages(aDevice);
    vkDestroyImageView(device, colorImageView, nullptr);
    aDevice->DestroyImage(colorImage, colorImageAllocation);

    for (auto imageView : swapChainImageViews)
        vkDestroyImageView(device, imageView, nullptr);
    for (size_t i = 0; i < depthImages.size(); i++)
    {
        vkDestroyImageView(device, depthImageViews[i], nullptr);
        aDevice->DestroyImage(depthImages[i], depthImageAllocations[i]);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}
