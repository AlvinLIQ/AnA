#include "Headers/CommandBuffer.hpp"

using namespace AnA;

CommandBuffer::CommandBuffer(Device& mDevice, VkCommandPool commandPool, VkCommandBufferBeginInfo commandBufferBeginInfo) : aDevice{mDevice}
{
    
}

CommandBuffer::~CommandBuffer()
{
    vkDestroyCommandPool(aDevice.GetLogicalDevice(), pool, nullptr);
}

void CommandBuffer::Begin()
{
    vkBeginCommandBuffer(buffer, &beginInfo);
}

void CommandBuffer::End()
{
    vkEndCommandBuffer(buffer);
}

VkCommandBuffer CommandBuffer::Get() const
{
    return buffer;
}

void CommandBuffer::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = level;
    vkAllocateCommandBuffers(aDevice.GetLogicalDevice(), &allocInfo, &buffer);
}