#pragma once
#include "../../Resources/Headers/Object.hpp"
#include "../../Headers/SwapChain.hpp"
#include <vulkan/vulkan_core.h>

namespace AnA
{
    namespace Systems
    {
        class RenderSystem
        {
        public:
            RenderSystem(Device& mDevice, SwapChain& mSwapChain);
            ~RenderSystem();

            void RenderObjects(VkCommandBuffer commandBuffer, Objects &objects, Shader& shader);
            static RenderSystem* GetCurrent();
        private:
            Device& aDevice;
            SwapChain& aSwapChain;
        };
    }
}