#pragma once
#include "../../Resources/Headers/Object.hpp"
#include "../../Resources/Headers/Mesh.hpp"
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

            void RenderObject(VkCommandBuffer commandBuffer, Object& object, Shader& shader);
            void RenderMeshes(VkCommandBuffer commandBuffer, Meshes& meshes, Shader& shader);
            void RenderBatch(VkCommandBuffer commandBuffer, Meshes& meshes, Shader& shader, size_t batchIndex);
            static RenderSystem* GetCurrent();
        private:
            Device& aDevice;
            SwapChain& aSwapChain;
        };
    }
}