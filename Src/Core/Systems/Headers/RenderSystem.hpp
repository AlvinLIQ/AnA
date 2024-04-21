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
            RenderSystem(Device& mDevice, SwapChain& mSwapChain);
            ~RenderSystem();

            void RenderShapes(VkCommandBuffer commandBuffer, Shapes& shapes, Shader& shader);
            void RenderShapesIndirect(VkCommandBuffer commandBuffer, Shapes& shapes, Shader& shader);
            void RenderMeshes(VkCommandBuffer commandBuffer, Meshes& meshes, Shader& shader);
            void RenderBatch(VkCommandBuffer commandBuffer, Meshes& meshes, Shader& shader, size_t batchIndex);
            static RenderSystem* GetCurrent();
        private:
            Device& aDevice;
            SwapChain& aSwapChain;
        };
    }
}