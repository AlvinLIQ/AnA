#include "Headers/CommandBuffer.hpp"
#include "../Headers/SwapChain.hpp"

#define NextBufferIndex ((currentBufferIndex + 1) % static_cast<uint32_t>(buffers.size()))

using namespace AnA;

CommandBuffer::CommandBuffer(Device* mDevice, int commandBufferCount, 
VkCommandBufferLevel commandBufferlevel, 
VkCommandBufferUsageFlags usageFlags, bool async) : aDevice{mDevice}, async{async}, level{commandBufferlevel}
{
    if (async)
    {
        aDevice->CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &pool);
    }
    else
    {
        pool = aDevice->GetCommandPool();
    }
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;
    beginInfo.pInheritanceInfo = nullptr;

    buffers.resize(commandBufferCount);
    nextBufferIndex = NextBufferIndex;
    createCommandBuffer();
}

CommandBuffer::CommandBuffer(Device* mDevice, int commandBufferCount, 
VkCommandBufferLevel commandBufferlevel, 
VkCommandBufferBeginInfo& commandBufferBeginInfo) : aDevice{mDevice}, async{false}, beginInfo{commandBufferBeginInfo}, level{commandBufferlevel}
{
    pool = aDevice->GetCommandPool();
    buffers.resize(commandBufferCount);
    createCommandBuffer();
}

CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(aDevice->GetLogicalDevice(), pool, 
        static_cast<uint32_t>(buffers.size()), buffers.data());
    if (async)
    {
        vkDestroyCommandPool(aDevice->GetLogicalDevice(), pool, nullptr);
    }
}

VkCommandBuffer& CommandBuffer::Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo)
{
    beginInfo.pInheritanceInfo = pInheritanceInfo;
    if (vkBeginCommandBuffer(buffers[nextBufferIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin command buffer!");
    SwapChain::GetCurrent()->SetViewport(*this);
    return buffers[nextBufferIndex];
}

VkCommandBuffer& CommandBuffer::Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D offset)
{
    beginInfo.pInheritanceInfo = pInheritanceInfo;
    if (vkBeginCommandBuffer(buffers[nextBufferIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin command buffer!");
    auto extent = SwapChain::GetCurrent()->GetExtent();
    VkViewport viewport = {(float)offset.x, (float)offset.y, static_cast<float>(extent.width - offset.x), 
        static_cast<float>(extent.height - offset.y), 0.0f, 1.0f};
    VkRect2D scissor = {offset, {extent.width - offset.x, extent.height - offset.y}};
    vkCmdSetViewport(buffers[nextBufferIndex], 0, 1, &viewport);
    vkCmdSetScissor(buffers[nextBufferIndex], 0, 1, &scissor);
    return buffers[nextBufferIndex];
}

VkCommandBuffer& CommandBuffer::Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D offset, VkExtent2D extent)
{
    beginInfo.pInheritanceInfo = pInheritanceInfo;
    if (vkBeginCommandBuffer(buffers[nextBufferIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin command buffer!");
    VkViewport viewport = {0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
    VkRect2D scissor = {offset, extent};
    vkCmdSetViewport(buffers[nextBufferIndex], 0, 1, &viewport);
    vkCmdSetScissor(buffers[nextBufferIndex], 0, 1, &scissor);
    return buffers[nextBufferIndex];
}

VkCommandBuffer& CommandBuffer::Begin(VkCommandBufferInheritanceInfo* pInheritanceInfo, VkOffset2D ltOffset, VkOffset2D rbOffset)
{
    beginInfo.pInheritanceInfo = pInheritanceInfo;
    if (vkBeginCommandBuffer(buffers[nextBufferIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin command buffer!");
    auto extent = SwapChain::GetCurrent()->GetExtent();
    VkViewport viewport = {0.0f, 0.0f, static_cast<float>(extent.width - ltOffset.x - rbOffset.x),
        static_cast<float>(extent.height - ltOffset.y - rbOffset.y), 0.0f, 1.0f};
    VkRect2D scissor = {ltOffset, extent};
    vkCmdSetViewport(buffers[nextBufferIndex], 0, 1, &viewport);
    vkCmdSetScissor(buffers[nextBufferIndex], 0, 1, &scissor);
    return buffers[nextBufferIndex];
}

void CommandBuffer::End()
{
    vkEndCommandBuffer(buffers[nextBufferIndex]);
    currentBufferIndex = nextBufferIndex;
    nextBufferIndex = NextBufferIndex;
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
    vkAllocateCommandBuffers(aDevice->GetLogicalDevice(), &allocInfo, buffers.data());
}