#pragma once
#include "../../Resources/Headers/Mesh.hpp"
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
            void RenderMeshes(CommandBuffer& commandBuffer, Meshes& meshes, Shader& shader, uint32_t bufferIndex);
            void RenderMeshesIndirect(CommandBuffer& commandBuffer, Meshes& meshes, Shader& shader, uint32_t bufferIndex);
            static RenderSystem* GetCurrent();
        private:
            Device* aDevice;
            SwapChain& aSwapChain;
        };
    }
}