#pragma once
#include "../../Headers/Threadpool.hpp"
#include "CommandBuffer.hpp"

namespace AnA
{
    class CommandBufferPool : public ThreadPool<void(CommandBuffer*)>
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
        void Enqueue(RecordCallBack recordCallBack, VkCommandBufferInheritanceInfo* pInheritanceInfo)
        {
            ThreadPool::Enqueue([this, recordCallBack, pInheritanceInfo](CommandBuffer* commandBuffer)
            {
                auto& _commandBuffer = commandBuffer->Begin(pInheritanceInfo);
                recordCallBack(_commandBuffer);
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
    private:
        Device& aDevice;
        std::vector<CommandBuffer> commandBuffers;
        std::vector<VkCommandBuffer> recordedCommandBuffers;
    };
}