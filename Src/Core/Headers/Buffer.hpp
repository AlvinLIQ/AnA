#pragma once
#include "Device.hpp"
#include <string.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

namespace AnA
{
    class Buffer
    {
    public:
        Buffer(Device& mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
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
        const VkDeviceSize GetSize() const
        {
            return bufferSize;
        }

        void CopyToBuffer(Buffer& srcBuffer, VkDeviceSize bufferSize)
        {
            aDevice.CopyBuffer(srcBuffer.GetBuffer(), buffer, bufferSize);
        }

        void CopyToBuffer(Buffer& srcBuffer, uint32_t regionCount, const VkBufferCopy* regions)
        {
            aDevice.CopyBuffer(srcBuffer.GetBuffer(), buffer, regionCount, regions);
        }

        void CopyToBuffer(Buffer& srcBuffer, uint32_t regionCount, const VkBufferCopy* regions, VkCommandBuffer commandBuffer)
        {
            vkCmdCopyBuffer(commandBuffer, srcBuffer.GetBuffer(), buffer, regionCount, regions);
        }

        void CopyToBuffer(const void* data, VkDeviceSize bufferSize)
        {
            this->Map(0, bufferSize);
            memcpy(this->GetMappedData(), data, bufferSize);
            this->Unmap();
        }
    private:
        Device& aDevice;

        void* mappedData = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
        VkDeviceSize bufferSize = 0;
    };
}