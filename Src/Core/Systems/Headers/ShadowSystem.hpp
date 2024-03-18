#pragma once
#include "../../Headers/Pipeline.hpp"
#include "../../Headers/SwapChain.hpp"

namespace AnA
{
    struct PointLight
    {
        glm::vec3 position;
        glm::vec3 color;
    };
    struct Ray
    {
        glm::vec3 center;
        glm::vec3 direction;
    };
    namespace Systems
    {
        class ShadowSystem
        {
        public:
            ShadowSystem(Device& mDevice, SwapChain* pSwapchain);
            ~ShadowSystem();

            VkExtent2D GetExtent();
            void RenderShadows();
        private:
            Device& aDevice;
            SwapChain* swapChain;
            Pipeline* shadowPipeline;
            
            VkFormat shadowFormat{VK_FORMAT_UNDEFINED};
            VkImage image;
            VkDeviceMemory imageMemory;
            VkImageView imageView;
            void createImageView();
            VkRenderPass renderPass;
            void createShadowRenderPass();
            VkFramebuffer shadowFrameBuffer;
            void createShadowFrameBuffer();
        };
    }
}