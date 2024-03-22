#pragma once
#include "../../Headers/SwapChain.hpp"
#include "../../Headers/Object.hpp"

namespace AnA
{
    struct LightBufferObject
    {
        glm::mat4 proj{1.f};
        glm::mat4 view{1.f};
        alignas(16) glm::vec3 direction{0.0f};
        alignas(16) glm::vec3 color{};
        float ambient{0.0f};
    };
    namespace Cameras
    {
        class Camera;
    }
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

            VkSampler shadowSampler;
            void createShadowSampler();

            Descriptor* shadowDescriptor, *lightDescriptor;
            
            std::vector<Buffer*> lightBuffers;
            void createLightBuffers();
        };
    }
}