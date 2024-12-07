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
            ShadowSystem(Device* mDevice, SwapChain* pSwapchain);
            ~ShadowSystem();
            static ShadowSystem* GetCurrent();

            void RenderShadows(VkCommandBuffer commandBuffer, Meshes& meshes, Shader& shader);
            void RenderShadowsIndirect(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader);
            void RenderCascadedShadowsIndirect(VkCommandBuffer commandBuffer, Meshes &meshes, Shader& shader, uint32_t& index);
        private:
            Device* aDevice;
            SwapChain* swapChain;

            VkClearValue clearValue{.depthStencil{1.0f, 0}};
        };
    }
}