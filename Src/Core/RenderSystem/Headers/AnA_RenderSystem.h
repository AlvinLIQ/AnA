#pragma once
#include "../../Headers/AnA_Object.h"
#include "../../Headers/AnA_Pipeline.h"
#include "../../Headers/AnA_SwapChain.h"
#include "../../Headers/AnA_Camera.h"
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
            static void CreateBindingDescriptionSet(VkDescriptorSetLayoutBinding* pDescSet);
            static VkDescriptorBufferInfo GetBufferInfo(VkBuffer camBuffer);
        };
        
        class AnA_RenderSystem
        {
        public:
            AnA_RenderSystem(AnA_Device *&mDevice, AnA_SwapChain *&mSwapChain);
            ~AnA_RenderSystem();

            void RenderObjects(VkCommandBuffer commandBuffer, std::vector<AnA_Object*> &objects, AnA_Camera &camera);

        private:
            AnA_Device *&aDevice;
            AnA_SwapChain *&aSwapChain;
            
            VkPipelineLayout pipelineLayout;
            void createPipelineLayout();
            
            AnA_Pipeline *aPipeline;

            CameraBufferObject cameraBuffer;
            std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> descriptionSetLayouts;
        };
    }
}