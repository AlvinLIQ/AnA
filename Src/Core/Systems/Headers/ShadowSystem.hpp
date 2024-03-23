#pragma once
#include "../../Headers/SwapChain.hpp"
#include "../../Resources/Headers/Object.hpp"

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
            std::vector<VkDescriptorSet>& GetShadowSamplerSets();
            std::vector<VkDescriptorSet>& GetLightSets();

            void BeginRenderPass(VkCommandBuffer& commandBuffer);
            void EndRenderPass(VkCommandBuffer& commandBuffer);

            void UpdateLightBuffer(Cameras::Camera& lightCamera);
        private:
            Device& aDevice;
            SwapChain* swapChain;
            
            VkImage image;
            VkDeviceMemory imageMemory;
            VkImageView imageView;
            void createImageView();
            
            VkFramebuffer shadowFrameBuffer;
            void createShadowFrameBuffer();

            Descriptor* shadowDescriptor, *lightDescriptor;
            
            std::vector<Buffer*> lightBuffers;
            void createLightBuffers();
        };
    }
}