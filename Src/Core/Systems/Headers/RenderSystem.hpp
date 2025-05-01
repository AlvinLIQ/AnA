#pragma once
#include "../../Resources/Headers/Renderable.hpp"
#include <vulkan/vulkan_core.h>

namespace AnA
{
    namespace Systems
    {
        class RenderSystem
        {
        public:
            RenderSystem();
            ~RenderSystem();

            void RenderIndirect(CommandBuffer& commandBuffer, Renderable& renderable, Shader& shader, uint32_t bufferIndex = 0);
            static RenderSystem* GetCurrent();
        };
    }
}