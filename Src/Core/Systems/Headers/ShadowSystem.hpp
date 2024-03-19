#pragma once
#include "../../Headers/SwapChain.hpp"
#include "../../Headers/Object.hpp"

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
            void RenderShadows(VkCommandBuffer commandBuffer, Objects &objects);
            VkDescriptorSet& GetShadowSamplerSet();
        private:
            Device& aDevice;
            SwapChain* swapChain;
            
            VkImage image;
            VkDeviceMemory imageMemory;
            VkImageView imageView;
            void createImageView();
            
            VkFramebuffer shadowFrameBuffer;
            void createShadowFrameBuffer();

            VkSampler shadowSampler;
            void createShadowSampler();

            VkDescriptorPool descriptorPool;
            VkDescriptorSet descriptorSet;
            void createDescriptorSet();
        };
    }
}