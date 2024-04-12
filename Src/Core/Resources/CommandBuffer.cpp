#include "Headers/CommandBuffer.hpp"

#define NextBufferIndex ((currentBufferIndex + 1) % static_cast<uint32_t>(buffers.size()))

using namespace AnA;

CommandBuffer::CommandBuffer(Device& mDevice, int commandBufferCount, VkCommandBufferLevel commandBufferlevel, VkCommandBufferUsageFlags usageFlags) : aDevice{mDevice}, level{commandBufferlevel}
{
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;
    beginInfo.pInheritanceInfo = nullptr;

    buffers.resize(commandBufferCount);
    createCommandBuffer();
}

CommandBuffer::CommandBuffer(Device& mDevice, int commandBufferCount, VkCommandBufferLevel commandBufferlevel, VkCommandBufferBeginInfo& commandBufferBeginInfo) : aDevice{mDevice}, beginInfo{commandBufferBeginInfo}, level{commandBufferlevel}
{
    buffers.resize(commandBufferCount);
    createCommandBuffer();
}

CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(aDevice.GetLogicalDevice(), aDevice.GetCommandPool(), static_cast<uint32_t>(buffers.size()), buffers.data());
}

VkCommandBuffer& CommandBuffer::Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo)
{
    beginInfo.pInheritanceInfo = pInheritanceInfo;
    if (vkBeginCommandBuffer(buffers[NextBufferIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin command buffer!");
    return buffers[NextBufferIndex];
}

void CommandBuffer::End()
{
    int nextBufferIndex = NextBufferIndex;
    vkEndCommandBuffer(buffers[nextBufferIndex]);
    currentBufferIndex = nextBufferIndex;
}

const VkCommandBuffer& CommandBuffer::Get() const
{
    return buffers[currentBufferIndex];
}

void CommandBuffer::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = aDevice.GetCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
    allocInfo.level = level;
    vkAllocateCommandBuffers(aDevice.GetLogicalDevice(), &allocInfo, buffers.data());
}