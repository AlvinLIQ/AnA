#pragma once
#include "../../Headers/Threadpool.hpp"
#include "CommandBuffer.hpp"

namespace AnA
{
    class CommandBufferPool : public ThreadPool<void(CommandBuffer*, size_t index)>
    {
    public:
        CommandBufferPool(Device& mDevice, 
        VkCommandBufferLevel commandBufferLevel, 
        VkCommandBufferUsageFlags commandBufferUsage, 
        size_t count = std::thread::hardware_concurrency()) : aDevice{mDevice}, ThreadPool(&commandBuffers, count)
        {
            commandBuffers.reserve(count);
            for (size_t i = 0; i < count; i++)
                commandBuffers.emplace_back(mDevice, MAX_FRAMES_IN_FLIGHT, commandBufferLevel, commandBufferUsage, true);
        }
        ~CommandBufferPool()
        {
            commandBuffers.clear();
        }
        operator bool()
        {
            return recordedCommandBuffers.size() != 0;
        }
        void Reset()
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            recordedCommandBuffers.clear();
            CurrentBufferIndex = (CurrentBufferIndex + 1) % MAX_FRAMES_IN_FLIGHT;
        }
        void ExcuteRecordedBuffer(VkCommandBuffer& commandBuffer)
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            vkCmdExecuteCommands(commandBuffer, 
                static_cast<uint32_t>(recordedCommandBuffers.size()), 
                recordedCommandBuffers.data());
        }
        void Enqueue(RecordCallBackEx recordCallBack, VkCommandBufferInheritanceInfo* pInheritanceInfo)
        {
            ThreadPool::Enqueue([this, recordCallBack, pInheritanceInfo](CommandBuffer* commandBuffer, size_t index)
            {
                auto& _commandBuffer = commandBuffer->Begin(pInheritanceInfo);
                recordCallBack(_commandBuffer, index);
                commandBuffer->End();
                std::unique_lock<std::mutex> lock(queue_mutex_);
                recordedCommandBuffers.push_back(_commandBuffer);
            });
        }
        void Enqueue(RecordCallBackEx recordCallBack, VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D offset)
        {
            ThreadPool::Enqueue([this, recordCallBack, pInheritanceInfo, offset](CommandBuffer* commandBuffer, size_t index)
            {
                auto& _commandBuffer = commandBuffer->Begin(pInheritanceInfo, offset);
                recordCallBack(_commandBuffer, index);
                commandBuffer->End();
                std::unique_lock<std::mutex> lock(queue_mutex_);
                recordedCommandBuffers.push_back(_commandBuffer);
            });
        }
        void Enqueue(RecordCallBackEx recordCallBack, VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D offset, VkExtent2D extent)
        {
            ThreadPool::Enqueue([this, recordCallBack, pInheritanceInfo, offset, extent](CommandBuffer* commandBuffer, size_t index)
            {
                auto& _commandBuffer = commandBuffer->Begin(pInheritanceInfo, offset, extent);
                recordCallBack(_commandBuffer, index);
                commandBuffer->End();
                std::unique_lock<std::mutex> lock(queue_mutex_);
                recordedCommandBuffers.push_back(_commandBuffer);
            });
        }
        struct CommandBufferRecordedInfo
        {
            size_t index;
            size_t recordedIndex;
        };
        uint32_t CurrentBufferIndex = 0;
        uint32_t GetCommandBufferCount()
        {
            return static_cast<uint32_t>(commandBuffers.size());
        }
    private:
        Device& aDevice;
        std::vector<CommandBuffer> commandBuffers;
        std::vector<VkCommandBuffer> recordedCommandBuffers;
    };
}