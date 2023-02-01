#pragma once
#include "../../Headers/AnA_Object.h"
#include "../../Headers/AnA_Pipeline.h"
#include "../../Headers/AnA_SwapChain.h"
#include <vulkan/vulkan_core.h>

#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_CIRCLE 2
#define ANA_CURVED_RECTANGLE 3

namespace AnA
{
    namespace RenderSystems
    {
        class AnA_RenderSystem
        {
        public:
            AnA_RenderSystem(AnA_Device *&mDevice, AnA_SwapChain *&mSwapChain);
            ~AnA_RenderSystem();

            void RenderObjects(VkCommandBuffer commandBuffer, std::vector<AnA_Object*> &objects);

            //void CreateRectangleObject(glm::vec2 offset, glm::vec2 scalar, AnA_Object** object);

        private:
            AnA_Device *&aDevice;
            AnA_SwapChain *&aSwapChain;
            
            VkPipelineLayout pipelineLayout;
            void createPipelineLayout();
            
            AnA_Pipeline *aPipeline;
        };
    }
}