#include "Headers/ShadowSystem.hpp"
#include "../Resources/Headers/ResourceManager.hpp"

using namespace AnA;
using namespace Systems;

ShadowSystem* _shadowSystem;

ShadowSystem::ShadowSystem(Device& mDevice, SwapChain* pSwapchain) : aDevice(mDevice)
{
    swapChain = pSwapchain;
    _shadowSystem = this;
}

ShadowSystem::~ShadowSystem()
{
    
}

ShadowSystem* ShadowSystem::GetCurrent()
{
    return _shadowSystem;
}

VkExtent2D ShadowSystem::GetExtent()
{
    return swapChain->GetExtent();
}

void ShadowSystem::RenderShadows(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader)
{
    vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
    shader.GetPipeline()->Bind(commandBuffer);
    std::vector<VkDescriptorSet> sets = shader.GetDescriptorSets()[swapChain->CurrentFrame];

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 2,
        sets.data(), 0, nullptr);
    meshes.Bind(commandBuffer);
    meshes.Draw(commandBuffer);
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

    auto extent = swapChain->GetExtent();

    VkRenderPassBeginInfo renderPassBegin;
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.pNext = NULL;
    renderPassBegin.renderPass = swapChain->GetOffscreenRenderPass();
    renderPassBegin.framebuffer = Resource::ResourceManager::GetCurrent()->GetShadowFramebuffers()[swapChain->CurrentFrame];
    renderPassBegin.renderArea.offset.x = 0;
    renderPassBegin.renderArea.offset.y = 0;
    renderPassBegin.renderArea.extent = {extent.width, extent.height};
    renderPassBegin.clearValueCount = 1;
    renderPassBegin.pClearValues = &clearValue;
    
    vkCmdBeginRenderPass(commandBuffer,
                        &renderPassBegin,
                        VK_SUBPASS_CONTENTS_INLINE);

    swapChain->SetViewport(commandBuffer);
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
