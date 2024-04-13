#include "Headers/CommandBuffer.hpp"
#include "../Headers/SwapChain.hpp"

#define NextBufferIndex ((currentBufferIndex + 1) % static_cast<uint32_t>(buffers.size()))

using namespace AnA;

CommandBuffer::CommandBuffer(Device& mDevice, int commandBufferCount, 
VkCommandBufferLevel commandBufferlevel, 
VkCommandBufferUsageFlags usageFlags, bool async) : aDevice{mDevice}, level{commandBufferlevel}, async{async}
{
    if (async)
    {
        aDevice.CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &pool);
    }
    else
    {
        pool = aDevice.GetCommandPool();
    }
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;
    beginInfo.pInheritanceInfo = nullptr;

    buffers.resize(commandBufferCount);
    createCommandBuffer();
}

CommandBuffer::CommandBuffer(Device& mDevice, int commandBufferCount, 
VkCommandBufferLevel commandBufferlevel, 
VkCommandBufferBeginInfo& commandBufferBeginInfo) : aDevice{mDevice}, beginInfo{commandBufferBeginInfo}, level{commandBufferlevel}, async{false}
{
    pool = aDevice.GetCommandPool();
    buffers.resize(commandBufferCount);
    createCommandBuffer();
}

CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(aDevice.GetLogicalDevice(), pool, 
        static_cast<uint32_t>(buffers.size()), buffers.data());
    if (async)
    {
        vkDestroyCommandPool(aDevice.GetLogicalDevice(), pool, nullptr);
    }
}

VkCommandBuffer& CommandBuffer::Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo)
{
    beginInfo.pInheritanceInfo = pInheritanceInfo;
    if (vkBeginCommandBuffer(buffers[NextBufferIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin command buffer!");
    SwapChain::GetCurrent()->SetViewport(buffers[NextBufferIndex]);
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
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
    allocInfo.level = level;
    vkAllocateCommandBuffers(aDevice.GetLogicalDevice(), &allocInfo, buffers.data());
}