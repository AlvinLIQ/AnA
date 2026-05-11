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

void Buffer::cleanup()
{
    if (aDevice)
    {
        Unmap();
        aDevice->DestroyBuffer(buffer, allocation);
    }
}
