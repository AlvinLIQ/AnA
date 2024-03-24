#pragma once
#include "../../Headers/SwapChain.hpp"
#include "../../Resources/Headers/Object.hpp"

namespace AnA
{
    namespace Systems
    {
        class ShadowSystem
        {
        public:
            ShadowSystem(Device& mDevice, SwapChain* pSwapchain);
            ~ShadowSystem();
            static ShadowSystem* GetCurrent();

            VkExtent2D GetExtent();
            void RenderShadows(VkCommandBuffer commandBuffer, Objects& objects, Shader& shader);

            void BeginRenderPass(VkCommandBuffer& commandBuffer);
            void EndRenderPass(VkCommandBuffer& commandBuffer);
        private:
            Device& aDevice;
            SwapChain* swapChain;
        };
    }
}