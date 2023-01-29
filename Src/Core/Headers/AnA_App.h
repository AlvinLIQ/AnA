#pragma once

#include "AnA_Object.h"
#include "AnA_Renderer.h"
#include "AnA_Window.h"
#include "AnA_Instance.h"
#include "AnA_Device.h"
#include "AnA_Pipeline.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace AnA
{
    class AnA_App
    {
    public:
        AnA_App();
        ~AnA_App();

        void Init();
        void Run();
        void Cleanup();
        void Exit();

    private:
        AnA_Window* aWindow;
        AnA_Instance* aInstance;
        AnA_Device* aDevice;
        AnA_Pipeline* aPipeline;
        AnA_Renderer* aRenderer;

        VkPipelineLayout pipelineLayout;
        void createPipelineLayout();

        std::vector<AnA_Object*> objects;
        void loadObjects();
        void renderObjects(VkCommandBuffer commandBuffer);
    };
}
