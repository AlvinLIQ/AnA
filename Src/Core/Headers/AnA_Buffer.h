#pragma once
#include "AnA_Device.h"

namespace AnA
{
    class AnA_Buffer
    {
    public:
        AnA_Buffer(AnA_Device *&mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~AnA_Buffer();
        VkResult Map(VkDeviceSize offset, VkDeviceSize size);
        void Unmap();

        VkBuffer GetBuffer()
        {
            return buffer;
        }
        VkDeviceMemory GetBufferMemory()
        {
            return bufferMemory;
        }
        void *GetMappedData()
        {
            return mappedData;
        }
    private:
        AnA_Device *&aDevice;

        void *mappedData = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    };
}