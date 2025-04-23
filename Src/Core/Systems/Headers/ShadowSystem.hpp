#pragma once
#include "../../Headers/SwapChain.hpp"
#include "../../Resources/Headers/Scene.hpp"

namespace AnA
{
    namespace Systems
    {
        class ShadowSystem
        {
        public:
            ShadowSystem(SwapChain* pSwapchain);
            ~ShadowSystem();
            static ShadowSystem* GetCurrent();

            void RenderShadows(CommandBuffer& commandBuffer, Scene& meshes, Shader& shader);
            void RenderShadowsIndirect(CommandBuffer& commandBuffer, Scene &meshes, Shader& shader);
            void RenderCascadedShadowsIndirect(CommandBuffer& commandBuffer, Scene &meshes, Shader& shader, uint32_t& index);
        private:
            //Device* aDevice;
            SwapChain* swapChain;

            //VkClearValue clearValue{.depthStencil{1.0f, 0}};
        };
    }
}