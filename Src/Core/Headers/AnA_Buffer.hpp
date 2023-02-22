#pragma once
#include "AnA_Device.hpp"
#include <vulkan/vulkan_core.h>
#include <cstring>

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

        void CopyToBuffer(const void *data, VkDeviceSize bufferSize)
        {
            this->Map(0, bufferSize);
            memcpy(this->GetMappedData(), data, bufferSize);
            this->Unmap();
        }
    private:
        AnA_Device *&aDevice;

        void *mappedData = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    };
}