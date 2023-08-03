#pragma once
#include "../../Headers/Object.hpp"
#include "../../Headers/Pipeline.hpp"
#include "../../Headers/SwapChain.hpp"
#include "../../Camera/Headers/Camera.hpp"
#include "../../Headers/Buffer.hpp"
#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>

namespace AnA
{
    namespace RenderSystems
    {
        struct CameraBufferObject
        {
            glm::mat4 proj{1.f};
            glm::mat4 view{1.f};
            static VkDescriptorSetLayoutBinding GetBindingDescriptionSet();
            static VkDescriptorBufferInfo GetBufferInfo(VkBuffer camBuffer);
        };
        
        class RenderSystem
        {
        public:
            RenderSystem(Device& mDevice, SwapChain& mSwapChain);
            ~RenderSystem();

            void RenderObjects(VkCommandBuffer commandBuffer, std::vector<Object*> &objects, Cameras::Camera &camera);
            void UpdateCameraBuffer(Cameras::Camera &camera);
            static RenderSystem* GetCurrent();

            std::array<VkDescriptorSetLayout, 2>& GetDescriptorSetLayouts();
        private:
            Device& aDevice;
            SwapChain& aSwapChain;
            
            VkPipelineLayout pipelineLayout;
            void createPipelineLayout();
            
            Pipeline* aGraphicsPipeline;

            std::vector<Buffer*> cameraBuffers;
            void createCameraBuffers();

            VkDescriptorPool descriptorPool;
            void createDescriptorPool();

            std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {};
            void createDescriptorSetLayout();
            std::vector<VkDescriptorSet> descriptorSets;
            void createDescriptorSets();
        };
    }
}