#pragma once
#include "Device.hpp"
#include <vulkan/vulkan_core.h>
#include <cstring>

namespace AnA
{
    class Buffer
    {
    public:
        Buffer(Device*& mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~Buffer();
        Buffer(const Buffer&) = delete;
        Buffer &operator=(const Buffer&) = delete;
        VkResult Map(VkDeviceSize offset, VkDeviceSize size);
        void Unmap();

        void UpdateData(void* newData, size_t dataSize);

        VkBuffer &GetBuffer()
        {
            return buffer;
        }
        VkDeviceMemory GetBufferMemory()
        {
            return bufferMemory;
        }
        void* GetMappedData()
        {
            return mappedData;
        }

        void CopyToBuffer(const void* data, VkDeviceSize bufferSize)
        {
            this->Map(0, bufferSize);
            memcpy(this->GetMappedData(), data, bufferSize);
            this->Unmap();
        }
    private:
        Device*& aDevice;

        void* mappedData = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    };
}