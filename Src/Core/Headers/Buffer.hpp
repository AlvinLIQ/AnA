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
        Buffer()
        {

        }
        Buffer(Device* mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage);
        ~Buffer();
//        Buffer(const Buffer&) = delete;
//        Buffer &operator=(const Buffer&) = delete;
        Buffer(Buffer&& buf) noexcept
        {
            cleanup();
            aDevice = buf.aDevice;
            mappedData = buf.mappedData;
            buffer = buf.buffer;
            bufferSize = buf.bufferSize;
            bufferUsage = buf.bufferUsage;
            memoryUsage = buf.memoryUsage;
            allocation = buf.allocation;
            newBufferRecords = buf.newBufferRecords;
            buf.mappedData = nullptr;
            buf.buffer = VK_NULL_HANDLE;
            buf.allocation = nullptr;
        }
        Buffer &operator=(Buffer&& buf) noexcept
        {
            if (&buf != this)
            {
                cleanup();
                aDevice = buf.aDevice;
                mappedData = buf.mappedData;
                buffer = buf.buffer;
                bufferSize = buf.bufferSize;
                bufferUsage = buf.bufferUsage;
                memoryUsage = buf.memoryUsage;
                allocation = buf.allocation;
                newBufferRecords = buf.newBufferRecords;
                buf.mappedData = nullptr;
                buf.buffer = VK_NULL_HANDLE;
                buf.allocation = nullptr;
            }
            return *this;
        }
        void Map();
        void Unmap();

        void UpdateData(void* newData, size_t dataSize);
        void Flush();

        VkBuffer GetBuffer();
        void* GetMappedData()
        {
            return mappedData;
        }
        VkDeviceSize GetSize() const
        {
            return bufferSize;
        }

        void Resize(VkDeviceSize newSize)
        {
            if (newSize != bufferSize)
            {
                *this = Buffer(aDevice, newSize, bufferUsage, memoryUsage);
            }
        }

        void CopyToBuffer(Buffer& srcBuffer, VkDeviceSize dataSize)
        {
            aDevice->CopyBuffer(srcBuffer.GetBuffer(), buffer, dataSize);
        }

        void CopyToBuffer(Buffer& srcBuffer, uint32_t regionCount, const VkBufferCopy* regions)
        {
            aDevice->CopyBuffer(srcBuffer.GetBuffer(), buffer, regionCount, regions);
        }

        void CopyToBuffer(Buffer& srcBuffer, uint32_t regionCount, const VkBufferCopy* regions, VkCommandBuffer commandBuffer)
        {
            vkCmdCopyBuffer(commandBuffer, srcBuffer.GetBuffer(), buffer, regionCount, regions);
        }

        void CopyToBuffer(const void* data, VkDeviceSize dataSize)
        {
            this->Map();
            memcpy(this->GetMappedData(), data, dataSize);
            this->Unmap();
        }
    private:
        Device* aDevice = nullptr;

        void* mappedData = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize bufferSize = 0;
        VkBufferUsageFlags bufferUsage;
        VmaMemoryUsage memoryUsage;
        VmaAllocation allocation;

        int newBufferRecords = 0;

        void replace();
        void cleanup();
    };
}