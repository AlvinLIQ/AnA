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
    shadowDescriptor = new Descriptor(mDevice, shadowSampler, imageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, MAX_FRAMES_IN_FLIGHT, Pipelines::GetCurrent()->GetDescriptorSetLayouts()[SHADOW_SAMPLER_LAYOUT], VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    createLightBuffers();
    lightDescriptor = new Descriptor(mDevice, lightBuffers.data(), sizeof(LightBufferObject), 0, MAX_FRAMES_IN_FLIGHT, Pipelines::GetCurrent()->GetDescriptorSetLayouts()[LIGHT_LAYOUT], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    createShadowFrameBuffer();
    _shadowSystem = this;
}

ShadowSystem::~ShadowSystem()
{
    delete shadowDescriptor;
    delete lightDescriptor;
    for (auto& lightBuffer : lightBuffers)
        delete lightBuffer;
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

    std::vector<VkDescriptorSet> sets{RenderSystem::GetCurrent()->GetDescriptorSet(), objects.GetDescriptorSets()[swapChain->CurrentFrame], lightDescriptor->GetSets()[swapChain->CurrentFrame]};
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelines->Get()[SHADOW_MAPPING_PIPELINE]->GetLayout(), 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);
    for (int i = 0; i < objectArray.size(); i++)
    {
        object = objectArray[i];
        if (object->Properties.sType != ANA_MODEL || i == 2)
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

std::vector<VkDescriptorSet>& ShadowSystem::GetShadowSamplerSets()
{
    return shadowDescriptor->GetSets(); 
}

std::vector<VkDescriptorSet>& ShadowSystem::GetLightSets()
{
    return lightDescriptor->GetSets();
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

void ShadowSystem::createLightBuffers()
{
    VkDeviceSize bufferSize = sizeof(LightBufferObject);
    lightBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto &lightBuffer : lightBuffers)
    {
        lightBuffer = new Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        lightBuffer->Map(0, bufferSize);
        LightBufferObject& lbo = *((LightBufferObject*)lightBuffer->GetMappedData());
        lbo.proj = glm::mat4{1.0f};
        lbo.view = glm::mat4{1.0f};
        lbo.direction = glm::normalize(glm::vec3(1.0f, -3.0f, 1.0f));
        lbo.color = glm::vec3(0.2);
        lbo.ambient = 0.037f;
        //printf("%p\n", lightBuffer->GetMappedData());
    }
}

void ShadowSystem::UpdateLightBuffer(Cameras::Camera& lightCamera)
{
    LightBufferObject& lbo = *((LightBufferObject*)lightBuffers[swapChain->CurrentFrame]->GetMappedData());
    lbo.proj = lightCamera.GetProjectionMatrix();
    lbo.view = lightCamera.GetView();
    //lightBuffers[swapChain->CurrentFrame]->CopyToBuffer(&lbo, sizeof(lbo));
}