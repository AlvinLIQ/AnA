#include "Headers/CommandBuffer.hpp"

using namespace AnA;

CommandBuffer::CommandBuffer(Device& mDevice, VkCommandBufferUsageFlags usageFlags, VkCommandBufferInheritanceInfo* pInheritInfo) : aDevice{mDevice}
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;
    beginInfo.pInheritanceInfo = pInheritInfo;
}

CommandBuffer::CommandBuffer(Device& mDevice, VkCommandBufferBeginInfo& commandBufferBeginInfo) : aDevice{mDevice}, beginInfo{commandBufferBeginInfo}
{

}

CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(aDevice.GetLogicalDevice(), aDevice.GetCommandPool(), 1, &buffer);
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
    allocInfo.commandPool = aDevice.GetCommandPool();
    allocInfo.commandBufferCount = 1;
    allocInfo.level = level;
    vkAllocateCommandBuffers(aDevice.GetLogicalDevice(), &allocInfo, &buffer);
}