#include "Headers/Buffer.hpp"
#include "Headers/Device.hpp"
#include <cassert>

using namespace AnA;

std::vector<Buffer*> replaceList{};

Buffer::Buffer(Device* mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage,
    VkDeviceSize alignment) : aDevice{mDevice},
 bufferSize {size}, bufferUsage{usage}, memoryUsage{memUsage}
{
    if (size)
    {
        aDevice->CreateBuffer(bufferSize + alignment, usage, memUsage, buffer, allocation);

        VkBufferDeviceAddressInfo addressInfo{};
        addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        addressInfo.buffer = buffer;

        address = vkGetBufferDeviceAddress(aDevice->GetLogicalDevice(), &addressInfo);
        if (alignment > 1)
            bufferOffset = AlignTo(address, alignment) - address;
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
    return address + bufferOffset;
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
