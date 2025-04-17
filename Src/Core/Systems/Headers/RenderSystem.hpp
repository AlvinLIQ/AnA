#pragma once
#include "../../Resources/Headers/Renderable.hpp"
#include "../../Resources/Headers/Shape.hpp"
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

            void RenderShapes(CommandBuffer& commandBuffer, Shapes& shapes, Shader& shader);
            void RenderShapesIndirect(CommandBuffer& commandBuffer, Shapes& shapes, Shader& shader);
            void RenderIndirect(CommandBuffer& commandBuffer, Renderable& renderable, uint32_t bufferIndex);
            static RenderSystem* GetCurrent();
        private:
            Device* aDevice;
            SwapChain& aSwapChain;
        };
    }
}