#include "Headers/Buffer.hpp"
#include <stdexcept>
#include <cassert>

using namespace AnA;
Buffer::Buffer(Device& mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : aDevice {mDevice}
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(aDevice.GetLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(aDevice.GetLogicalDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = aDevice.FindMemoryType(memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(aDevice.GetLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(aDevice.GetLogicalDevice(), buffer, bufferMemory, 0);
}

Buffer::~Buffer()
{
    vkDestroyBuffer(aDevice.GetLogicalDevice(), buffer, nullptr);
    vkFreeMemory(aDevice.GetLogicalDevice(), bufferMemory, nullptr);
}

VkResult Buffer::Map(VkDeviceSize offset, VkDeviceSize size)
{
    assert(buffer && bufferMemory && "Called map on buffer before create");
    return vkMapMemory(aDevice.GetLogicalDevice(), bufferMemory, offset, size, 0, &mappedData);
}

void Buffer::Unmap()
{
    if (mappedData)
    {
        vkUnmapMemory(aDevice.GetLogicalDevice(), bufferMemory);
        mappedData = nullptr;
    }
}

