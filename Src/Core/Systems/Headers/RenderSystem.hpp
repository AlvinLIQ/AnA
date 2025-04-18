#pragma once
#include "../../Resources/Headers/Renderable.hpp"
#include "../../Headers/SwapChain.hpp"
#include <vulkan/vulkan_core.h>

namespace AnA
{
    namespace Systems
    {
        class RenderSystem
        {
        public:
            RenderSystem(Device* mDevice, SwapChain& mSwapChain);
            ~RenderSystem();

            void RenderIndirect(CommandBuffer& commandBuffer, Renderable& renderable, uint32_t bufferIndex = 0);
            static RenderSystem* GetCurrent();
        private:
            Device* aDevice;
            SwapChain& aSwapChain;
        };
    }
}