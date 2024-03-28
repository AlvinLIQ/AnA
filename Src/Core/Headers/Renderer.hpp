#pragma once

#include "Device.hpp"
#include "Window.hpp"
#include "SwapChain.hpp"

#include <cassert>
#include <vector>
#include <vulkan/vulkan.h>

namespace AnA
{
    class Renderer
    {
    public:
        Renderer(Window& mWindow, Device& mDevice);
        ~Renderer();

        int GetFrameIndex() const
        {
            return currentFrameIndex;
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
            return commandBuffers[currentFrameIndex];
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

        VkCommandBuffer BeginFrame();

        void RecordSecondaryCommandBuffers(void(*recordCallBack)(VkCommandBuffer commandBuffer));
        void ExcuteSecondaryCommandBuffer(VkCommandBuffer commandBuffer);

        void EndFrame();
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D offset);
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkOffset2D offset, VkExtent2D extent);

        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        Window& aWindow;
        Device& aDevice;
        SwapChain* aSwapChain;

        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkCommandBuffer> secondaryCommandBuffers;
        void createCommandBuffers();
        void freeCommandBuffersMemory();

        uint32_t currentImageIndex = 0;
        int currentFrameIndex = 0;
        bool isFrameStarted = false;

        bool needUpdate = false;
        VkClearValue clearValues[2] = {{.color = {{0.6f, 0.6f, 0.6f, 1.0f}}}, {.depthStencil = {1.0f, 0}}};
    };
}
