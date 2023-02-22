#pragma once

#include "AnA_Device.hpp"
#include "AnA_Window.hpp"
#include "AnA_SwapChain.hpp"

#include <cassert>
#include <vector>
#include <vulkan/vulkan.h>

namespace AnA
{
    class AnA_Renderer
    {
    public:
        AnA_Renderer(AnA_Window*& mWindow, AnA_Device*& mDevice);
        ~AnA_Renderer();

        int GetFrameIndex() const
        {
            return currentFrameIndex;
        }

        bool IsFrameInProgress() const
        {
            return isFrameStarted;
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

        AnA_SwapChain *&GetSwapChain()
        {
            return aSwapChain;
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
        void EndFrame();
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        AnA_Window*& aWindow;
        AnA_Device*& aDevice;
        AnA_SwapChain* aSwapChain;

        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        std::vector<VkCommandBuffer> commandBuffers;
        void createCommandBuffers();
        void freeCommandBuffersMemory();

        uint32_t currentImageIndex = 0;
        int currentFrameIndex = 0;
        bool isFrameStarted = false;
    };
}
