#include "Headers/Buffer.hpp"
#include <cassert>

using namespace AnA;

std::vector<Buffer*> replaceList{};

Buffer::Buffer(Device* mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage) : aDevice{mDevice},
 bufferSize {size}, bufferUsage{usage}, memoryUsage{memUsage}
{
    if (bufferSize)
    {
        aDevice->CreateBuffer(size, usage, memUsage, buffer, allocation);

        VkBufferDeviceAddressInfo addressInfo{};
        addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        addressInfo.buffer = buffer;

        address = vkGetBufferDeviceAddress(aDevice->GetLogicalDevice(), &addressInfo);
    }
}

Buffer::~Buffer()
{
    cleanup();
}

void Buffer::Map()
{
    if (!mappedData)
        aDevice->MapBuffer(&mappedData, allocation);
}

void Buffer::Unmap()
{
    if (mappedData)
    {
        aDevice->UnmapBuffer(allocation);
        mappedData = nullptr;
    }
}

void Buffer::Flush()
{
    aDevice->FlushAllocation(allocation);
}

VkBuffer Buffer::GetBuffer()
{
    return buffer;
}

VkDeviceAddress Buffer::GetAddress()
{
    return address;
}

void Buffer::cleanup()
{
    if (aDevice)
    {
        Unmap();
        BufferResourceInfo info{buffer, allocation};
        Garbage garbage;
        garbage.info = info;
        garbage.cleanTime = aDevice->FrameIndex + MAX_FRAMES_IN_FLIGHT;
        aDevice->DumpGarbage(garbage);
    }
}
