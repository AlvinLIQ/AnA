#pragma once

#include "Device.hpp"
#include "Window.hpp"
#include "SwapChain.hpp"
#include "../Resources/Headers/CommandBuffer.hpp"

#include <cassert>
#include <vulkan/vulkan.h>

namespace AnA
{
    class Renderer
    {
    public:
        Renderer(Window& mWindow, Device* mDevice);
        ~Renderer();

        int GetFrameIndex() const
        {
            return aSwapChain->CurrentFrame;
        }

        bool IsFrameInProgress() const
        {
            return isFrameStarted;
        }

        bool NeedUpdate() const
        {
            return needUpdate;
        }

        VkRenderPass GetSwapChainRenderPass() const 
        {
            return aSwapChain->GetRenderPass();
        }
        VkCommandBuffer GetCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress!");
            return commandBuffers.Get();
        }

        SwapChain& GetSwapChain()
        {
            return *aSwapChain;
        }
        VkExtent2D GetSwapChainExtent() const
        {
            return aSwapChain->GetExtent();
        }
        float GetAspect() const
        {
            auto swapChainextent = GetSwapChainExtent();
            return static_cast<float>(swapChainextent.width) / static_cast<float>(swapChainextent.height);
        }
        VkCommandBufferInheritanceInfo& GetInheritanceInfo(RenderPassType renderPassType)
        {
            return inheritanceInfos[renderPassType];
        }
        VkCommandBuffer BeginFrame();

        void RecordOffscreenSecondaryCommandBuffer(RecordCallBack recordCallBack);
        void ExecuteOffscreenSecondaryCommandBuffer(VkCommandBuffer commandBuffer);

        void EndFrame();
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& offset);
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& offset, VkExtent2D& extent);
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D& ltOffset, VkOffset2D& rbOffset);

        void EndRenderPass(VkCommandBuffer commandBuffer);
        void BeginOffscreenRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer& framebuffer, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    private:
        Window& aWindow;
        Device* aDevice;
        SwapChain* aSwapChain;

        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT};
        CommandBuffer commandBuffers;
        CommandBuffer offscreenSecondaryCommandBuffers;

        uint32_t currentImageIndex = 0;
        bool isFrameStarted = false;

        bool needUpdate = false;
        VkClearValue clearValues[2] = {{.color = {{0.9f, 0.9f, 0.9f, 1.0f}}}, {.depthStencil = {1.0f, 0}}};
        VkCommandBufferInheritanceInfo inheritanceInfos[RENDER_PASS_TYPE_SIZE]{};
    };
}
