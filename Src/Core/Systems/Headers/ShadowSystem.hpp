#pragma once
#include "../../Headers/SwapChain.hpp"
#include "../../Resources/Headers/Mesh.hpp"

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
            void RenderShadows(VkCommandBuffer commandBuffer, Meshes& meshes, Shader& shader);
            int CurrentShadowIndex = 0;
        private:
            Device& aDevice;
            SwapChain* swapChain;

            VkClearValue clearValue{.depthStencil{1.0f, 0}};
        };
    }
}