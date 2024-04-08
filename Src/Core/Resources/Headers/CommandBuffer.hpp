#pragma once
#include "../../Headers/Device.hpp"

namespace AnA
{
    class CommandBuffer
    {
    public:
        CommandBuffer(Device& mDevice, VkCommandPool commandPool, VkCommandBufferBeginInfo commandBufferBeginInfo);
        ~CommandBuffer();

        void Begin();
        void End();
        VkCommandBuffer Get() const;
    private:
        Device& aDevice;
        VkCommandBufferBeginInfo beginInfo;
        VkCommandBuffer buffer;
        VkCommandBufferLevel level;
        void createCommandBuffer();
        VkCommandPool pool;
    };
}