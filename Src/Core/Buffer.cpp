#include "Headers/Buffer.hpp"
#include <cassert>

using namespace AnA;

std::vector<Buffer*> replaceList{};

Buffer::Buffer(Device* mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : aDevice{mDevice},
 bufferSize {size}, bufferUsage{usage}, bufferMemoryProperties{properties}
{
    if (bufferSize)
    {
        aDevice->CreateBuffer(size, usage, properties, buffer, bufferMemory);
    }
}

Buffer::~Buffer()
{
    cleanup();
    if (newBuffer)
        delete newBuffer;
}

VkResult Buffer::Map(VkDeviceSize offset, VkDeviceSize size)
{
    assert(buffer && bufferMemory && "Called map on buffer before create");
    return vkMapMemory(aDevice->GetLogicalDevice(), bufferMemory, offset, size, 0, &mappedData);
}

void Buffer::Unmap()
{
    if (mappedData)
    {
        vkUnmapMemory(aDevice->GetLogicalDevice(), bufferMemory);
        mappedData = nullptr;
    }
}

VkBuffer& Buffer::GetBuffer()
{
    if (this->newBuffer != nullptr)
    {
        replaceList.push_back(this);
        return newBuffer->buffer;
    }
    return buffer;
}

void Buffer::cleanup()
{
    if (aDevice)
    {
        vkDestroyBuffer(aDevice->GetLogicalDevice(), buffer, nullptr);
        vkFreeMemory(aDevice->GetLogicalDevice(), bufferMemory, nullptr);
    }
}

void Buffer::replace()
{
    cleanup();
    this->buffer = newBuffer->buffer;
    this->bufferMemory = newBuffer->bufferMemory;
    this->bufferSize = newBuffer->bufferSize;
    newBuffer = newBuffer->newBuffer;
}

void Buffer::ReplaceRequest(Buffer* newBuffer)
{
    while (this->newBuffer != nullptr);
    this->newBuffer = newBuffer;
}

void Buffer::TryReplace()
{
    if (replaceList.size())
    {
        //vkDeviceWaitIdle(replaceList[0]->aDevice->GetLogicalDevice());
        for (auto buffer = replaceList.begin(); buffer < replaceList.end(); buffer++)
        {
            (*buffer)->newBufferRecords++;
            if ((*buffer)->newBufferRecords > MAX_FRAMES_IN_FLIGHT + 1)
            {
                if ((*buffer)->newBuffer)
                {
                    (*buffer)->replace();
                    (*buffer)->newBufferRecords = 0;
                }
                replaceList.erase(buffer);
            }
        }
    }
}