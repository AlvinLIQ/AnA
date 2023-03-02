#pragma once
#include "../../Headers/AnA_Object.hpp"
#include "../../Headers/AnA_Pipeline.hpp"
#include "../../Headers/AnA_SwapChain.hpp"
#include "../../Camera/Headers/AnA_Camera.hpp"
#include "../../Headers/AnA_Buffer.hpp"
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
        
        class AnA_RenderSystem
        {
        public:
            AnA_RenderSystem(AnA_Device *&mDevice, AnA_SwapChain *&mSwapChain);
            ~AnA_RenderSystem();

            void RenderObjects(VkCommandBuffer commandBuffer, std::vector<AnA_Object*> &objects, Camera::AnA_Camera &camera);
            void UpdateCameraBuffer(Camera::AnA_Camera &camera);

        private:
            AnA_Device *&aDevice;
            AnA_SwapChain *&aSwapChain;
            
            VkPipelineLayout pipelineLayout;
            void createPipelineLayout();
            
            AnA_Pipeline *aPipeline;

            std::vector<AnA_Buffer*> cameraBuffers;
            void createCameraBuffers();

            VkDescriptorPool descriptorPool;
            void createDescriptorPool();
            std::vector<VkDescriptorSet> descriptorSets;
            void createDescriptorSets();

            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            void createDescriptorSetLayout();
        };
    }
}