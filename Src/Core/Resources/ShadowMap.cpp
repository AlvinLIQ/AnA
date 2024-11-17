#include "Headers/ShadowMap.hpp"
#include "../Headers/SwapChain.hpp"

using namespace AnA;
using namespace Resource;

ShadowMap::ShadowMap(Device* mDevice) : aDevice{mDevice}
{
    createShadowResources();
}

ShadowMap::~ShadowMap()
{
    cleanupShadowResources();
}

void ShadowMap::createShadowResources()
{
    shadowImages.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto& cascade : cascades)
    {
        cascade.imageViews.resize(shadowImages.size());
        cascade.framebuffers.resize(shadowImages.size());
    }
    bool samplersNotCreated = shadowSamplers.empty();
    if (samplersNotCreated)
        shadowSamplers.resize(shadowImages.size());
    auto swapChain = SwapChain::GetCurrent();
    auto extent = swapChain->GetExtent();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& shadowImage = shadowImages[i];
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_D32_SFLOAT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.extent = {extent.width, extent.height, 1};
        shadowImage.extent = imageInfo.extent;
        shadowImage.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        aDevice->CreateImage(&imageInfo, &shadowImage.image, &shadowImage.imageMemory);
        //aDevice->TransitionImageLayout(image, swapChain->GetDepthFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = shadowImage.image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = imageInfo.format;
        imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R };
        imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, SHADOW_MAP_CASCADE_COUNT };

        vkCreateImageView(aDevice->GetLogicalDevice(), &imageViewInfo, nullptr, &shadowImage.imageView);
        for (int j = 0; j < cascades.size(); j++)
        {
            auto& cascade = cascades[j];
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = swapChain->GetDepthFormat();
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = j; //layer index
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.image = shadowImages[i].image;
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &cascade.imageViews[i];

            framebufferInfo.width = imageInfo.extent.width;
            framebufferInfo.height = imageInfo.extent.height;
            framebufferInfo.layers = 1;
            vkCreateFramebuffer(aDevice->GetLogicalDevice(), &framebufferInfo, nullptr, &cascade.framebuffers[i]);
        }
        if (samplersNotCreated)
            aDevice->CreateSampler(&shadowSamplers[i], VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    }
}

void ShadowMap::cleanupShadowResources()
{
    for (auto& shadowSampler : shadowSamplers)
        vkDestroySampler(aDevice->GetLogicalDevice(), shadowSampler, nullptr);
    for (auto& cascade : cascades)
        cascade.cleanup(aDevice->GetLogicalDevice());
    for (auto& shadowImage : shadowImages)
        shadowImage.cleanup(aDevice->GetLogicalDevice());
}