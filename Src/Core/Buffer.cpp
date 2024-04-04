#include "Headers/Buffer.hpp"
#include <cassert>

using namespace AnA;
Buffer::Buffer(Device& mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : aDevice {mDevice},
 bufferSize {size}, bufferUsage{usage}, bufferMemoryProperties{properties}
{
    if (bufferSize)
    {
        aDevice.CreateBuffer(size, usage, properties, buffer, bufferMemory);
    }
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

