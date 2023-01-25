#pragma once

#include "AnA_Object.h"
#include "AnA_Window.h"
#include "AnA_Instance.h"
#include "AnA_Device.h"
#include "AnA_Surface.h"
#include "AnA_SwapChain.h"
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

        bool FramebufferResized = false;
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        AnA_Window* aWindow;
        AnA_Instance* aInstance;
        AnA_Device* aDevice;
        AnA_Surface* aSurface;
        AnA_SwapChain* aSwapChain;
        AnA_Pipeline* aPipeline;

        VkPipelineLayout pipelineLayout;
        void createPipelineLayout();

        VkCommandPool commandPool;
        void createCommandPool();

        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        std::vector<VkCommandBuffer> commandBuffers;
        void createCommandBuffer();

        void drawFrame();

        std::vector<AnA_Object*> objects;
        void loadObjects();
        void renderObjects(VkCommandBuffer commandBuffer);
    };
}
