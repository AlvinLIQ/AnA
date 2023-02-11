#pragma once
#include "../../Headers/AnA_Object.h"
#include "../../Headers/AnA_Pipeline.h"
#include "../../Headers/AnA_SwapChain.h"
#include "../../Headers/AnA_Camera.h"
#include <vulkan/vulkan_core.h>

namespace AnA
{
    namespace RenderSystems
    {
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
        };
    }
}