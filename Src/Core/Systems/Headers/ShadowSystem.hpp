#pragma once
#include "../../Headers/SwapChain.hpp"
#include "../../Resources/Headers/Renderable.hpp"

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

            void RenderCascadedShadowsIndirect(CommandBuffer& commandBuffer, Renderable& renderable, Shader& shader);
        private:
            //Device* aDevice;
            SwapChain* swapChain;

            //VkClearValue clearValue{.depthStencil{1.0f, 0}};
        };
    }
}