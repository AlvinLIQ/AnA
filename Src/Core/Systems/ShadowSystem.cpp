#include "Headers/ShadowSystem.hpp"
#include "Headers/RenderSystem.hpp"

using namespace AnA;
using namespace Systems;

ShadowSystem* _shadowSystem;

ShadowSystem::ShadowSystem(Device& mDevice, SwapChain* pSwapchain) : aDevice(mDevice)
{
    swapChain = pSwapchain;
    createImageView();
    createShadowSampler();
    createDescriptorSet();
    createShadowFrameBuffer();
    _shadowSystem = this;
}

ShadowSystem::~ShadowSystem()
{
    vkDestroyDescriptorPool(aDevice.GetLogicalDevice(), descriptorPool, nullptr);
    vkDestroySampler(aDevice.GetLogicalDevice(), shadowSampler, nullptr);
    vkDestroyImageView(aDevice.GetLogicalDevice(), imageView, nullptr);
    vkDestroyImage(aDevice.GetLogicalDevice(), image, nullptr);
    vkFreeMemory(aDevice.GetLogicalDevice(), imageMemory, nullptr);
    vkDestroyFramebuffer(aDevice.GetLogicalDevice(), shadowFrameBuffer, nullptr);
}

ShadowSystem* ShadowSystem::GetCurrent()
{
    return _shadowSystem;
}

VkExtent2D ShadowSystem::GetExtent()
{
    return swapChain->GetExtent();
}

void ShadowSystem::RenderShadows(VkCommandBuffer commandBuffer, Objects &objects)
{
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    auto pipelines = Pipelines::GetCurrent();
    pipelines->Get()[SHADOW_MAPPING_PIPELINE]->Bind(commandBuffer);
    Object* object;
    auto objectArray = objects.Get();

    std::vector<VkDescriptorSet> sets{RenderSystem::GetCurrent()->GetDescriptorSet(), objects.GetDescriptorSets()[swapChain->CurrentFrame]};
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelines->Get()[SHADOW_MAPPING_PIPELINE]->GetLayout(), 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);
    for (int i = 0; i < objectArray.size(); i++)
    {
        object = objectArray[i];
        if (object->Properties.sType != ANA_MODEL)
            continue;

        object->Model->Bind(commandBuffer);
        object->Model->Draw(commandBuffer, i);
    }
}

void ShadowSystem::BeginRenderPass(VkCommandBuffer& commandBuffer)
{
    /*
    vkDeviceWaitIdle(aDevice.GetLogicalDevice());
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
*/
    VkClearValue clearValues[1];
    clearValues[0].depthStencil.depth = 1.0f;
    clearValues[0].depthStencil.stencil = 0;

    auto extent = swapChain->GetExtent();

    VkRenderPassBeginInfo renderPassBegin;
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.pNext = NULL;
    renderPassBegin.renderPass = swapChain->GetOffscreenRenderPass();
    renderPassBegin.framebuffer = shadowFrameBuffer;
    renderPassBegin.renderArea.offset.x = 0;
    renderPassBegin.renderArea.offset.y = 0;
    renderPassBegin.renderArea.extent = extent;
    renderPassBegin.clearValueCount = 1;
    renderPassBegin.pClearValues = clearValues;
    
    vkCmdBeginRenderPass(commandBuffer,
                        &renderPassBegin,
                        VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport;
    viewport.height = extent.height;
    viewport.width = extent.width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    VkRect2D scissor;
    scissor.extent.width = extent.width;
    scissor.extent.height = extent.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void ShadowSystem::EndRenderPass(VkCommandBuffer& commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
 /*   vkEndCommandBuffer(commandBuffer);

    VkPipelineStageFlags shadow_map_wait_stages = 0;
    VkSubmitInfo submit_info = { };
    submit_info.pNext = NULL;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;
    submit_info.pWaitDstStageMask = 0;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(aDevice.GetGraphicsQueue(), 1, &submit_info, NULL);
*/
    //swapChain->CurrentFrame = (swapChain->CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    //vkQueueWaitIdle(aDevice.GetGraphicsQueue());
}

VkDescriptorSet& ShadowSystem::GetShadowSamplerSet()
{
    return descriptorSet; 
}

void ShadowSystem::createImageView()
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = swapChain->GetDepthFormat();
    auto extent = GetExtent();
	imageInfo.extent = { extent.width, extent.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    aDevice.CreateImage(&imageInfo, &image, &imageMemory);
    //aDevice.TransitionImageLayout(image, swapChain->GetDepthFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);

    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = imageInfo.format;
	imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R };
	imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

    vkCreateImageView(aDevice.GetLogicalDevice(), &imageViewInfo, nullptr, &imageView);
}

void ShadowSystem::createShadowFrameBuffer()
{
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = swapChain->GetOffscreenRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &imageView;

    auto extent = GetExtent();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;
    vkCreateFramebuffer(aDevice.GetLogicalDevice(), &framebufferInfo, nullptr, &shadowFrameBuffer);
}

void ShadowSystem::createShadowSampler()
{
    aDevice.CreateSampler(&shadowSampler, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
}

void ShadowSystem::createDescriptorSet()
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 2;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(aDevice.GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool!");

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = 1;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &Pipelines::GetCurrent()->GetDescriptorSetLayouts()[SHADOW_SAMPLER_LAYOUT];
    if (vkAllocateDescriptorSets(aDevice.GetLogicalDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = shadowSampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(aDevice.GetLogicalDevice(), 1,
        &descriptorWrite, 0, nullptr);
}