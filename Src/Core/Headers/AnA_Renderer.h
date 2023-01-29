#pragma once

#include "AnA_Device.h"
#include "AnA_Window.h"
#include "AnA_SwapChain.h"

#include <cassert>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

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
