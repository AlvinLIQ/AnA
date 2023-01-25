#pragma once

#include "AnA_Device.h"
#include "AnA_Pipeline.h"
#include "AnA_SwapChain.h"

#include <cstdint>
#include <stdexcept>
#include <array>

#include <vulkan/vulkan_core.h>

namespace AnA
{
    class AnA_Command
    {
    public:
        AnA_Command(AnA_Device* mDevice, AnA_SwapChain* mSwapChain, AnA_Pipeline* mPipeline)
        {
            aDevice = mDevice;
            aSwapChain = mSwapChain;
            aPipeline = mPipeline;
            createCommandPool();
            createCommandBuffer();
        }
        ~AnA_Command()
        {
            vkDestroyCommandPool(aDevice->GetLogicalDevice(), commandPool, nullptr);
        }

        void DrawFrame()
        {
            uint32_t imageIndex;
            auto result = aSwapChain->AcquireNextImage(&imageIndex);
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                aSwapChain->RecreateSwapChain();
                return;
            }
            else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                throw std::runtime_error("Failed to acquire swap chain image!");
            }

            recordCommandBuffer(imageIndex);
            result = aSwapChain->SubmitCommandBuffers(&commandBuffers[imageIndex],  &imageIndex); 

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                //FramebufferResized = false;
                aSwapChain->RecreateSwapChain();
            }
            else if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to present swap chain image!");
            }       
        }

    private:
        AnA_Device* aDevice;
        AnA_SwapChain* aSwapChain;
        AnA_Pipeline* aPipeline;
        VkCommandPool commandPool;
        void createCommandPool()
        {
            AnA_Device::QueueFamilyIndices queueFamilyIndices = aDevice->GetQueueFamiliesForCurrent();

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(aDevice->GetLogicalDevice(), &poolInfo, 
            nullptr, &commandPool) != VK_SUCCESS)
                throw std::runtime_error("Failed to create command pool!");
        }

        void recordCommandBuffer(uint32_t imageIndex)
        {
             VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
                    throw std::runtime_error("Failed to begin recording buffer!");
                
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = aSwapChain->GetRenderPass();
                renderPassInfo.framebuffer = aSwapChain->GetSwapChainFramebuffers()[imageIndex];

                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = aSwapChain->GetExtend();

                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0};
                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                
                aPipeline->Bind(commandBuffers[imageIndex]);
                vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);

                vkCmdEndRenderPass(commandBuffers[imageIndex]);
                if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to record command buffer!");
                }
        }

        std::vector<VkCommandBuffer> commandBuffers;
        void createCommandBuffer()
        {
            commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

            if (vkAllocateCommandBuffers(aDevice->GetLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
                throw std::runtime_error("Failed to allocate command buffers!");
        }

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
        {
            
        }
    };
}
