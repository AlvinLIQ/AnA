#pragma once
#include "../../Headers/Device.hpp"

namespace AnA
{
    class CommandBuffer
    {
    public:
        CommandBuffer(Device& mDevice, int commandBufferCount, VkCommandBufferLevel commandBufferlevel, VkCommandBufferUsageFlags usageFlags, bool async = false);
        CommandBuffer(Device& mDevice, int commandBufferCount, VkCommandBufferLevel commandBufferlevel, VkCommandBufferBeginInfo& commandBufferBeginInfo);
        ~CommandBuffer();

        VkCommandBuffer& Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr);
        VkCommandBuffer& Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D offset);
        VkCommandBuffer& Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D offset, VkExtent2D extent);
        VkCommandBuffer& Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D ltOffset, VkOffset2D rbOffset);
        void End();
        const VkCommandBuffer& Get() const;
    private:
        Device& aDevice;
        bool async;
        VkCommandPool pool{VK_NULL_HANDLE};
        VkCommandBufferBeginInfo beginInfo{};
        std::vector<VkCommandBuffer> buffers;
        VkCommandBufferLevel level;
        int currentBufferIndex = 0;
        void createCommandBuffer();
    };
}