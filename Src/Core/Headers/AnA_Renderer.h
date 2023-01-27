#pragma once

#include "AnA_Device.h"
#include "AnA_Object.h"
#include "AnA_Window.h"
#include "AnA_SwapChain.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace AnA
{
    class AnA_Renderer
    {
    public:
        AnA_Renderer(AnA_Window*& mWindow, AnA_Device*& mDevice);
        ~AnA_Renderer();

        void Cleanup();

        int GetFrameIndex() const
        {
            return currentFrameIndex;
        }

    private:
        AnA_Window*& aWindow;
        AnA_Device*& aDevice;
        AnA_SwapChain* aSwapChain;

        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        std::vector<VkCommandBuffer> commandBuffers;
        void createCommandBuffer();
        void freeCommandBuffersMemory();

        void drawFrame();

        std::vector<AnA_Object*> objects;

        uint32_t currentImageIndex = 0;
        int currentFrameIndex = 0;
    };
}
