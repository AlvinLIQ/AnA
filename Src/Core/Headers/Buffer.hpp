#pragma once
#include "Device.hpp"
#include "vulkan/vulkan_core.h"
#include <string.h>
#include <stdint.h>

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
            address = buf.address;
            allocation = buf.allocation;
            newBufferRecords = buf.newBufferRecords;
            buf.mappedData = nullptr;
            buf.address = 0;
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
                address = buf.address;
                allocation = buf.allocation;
                newBufferRecords = buf.newBufferRecords;
                buf.mappedData = nullptr;
                buf.buffer = VK_NULL_HANDLE;
                buf.address = 0;
                buf.allocation = nullptr;
            }
            return *this;
        }
        void Map();
        void Unmap();

        void UpdateData(void* newData, size_t dataSize);
        void Flush();

        VkBuffer GetBuffer();
        VkDeviceAddress GetAddress();
        void* GetMappedData()
        {
            return mappedData;
        }
        VkDeviceSize GetSize() const
        {
            return bufferSize;
        }
        VkBufferUsageFlags GetUsage() const
        {
            return bufferUsage;
        }

        void Resize(VkDeviceSize newSize)
        {
            if (newSize != bufferSize)
            {
                bool isMapped = mappedData;
                *this = Buffer(aDevice, newSize, bufferUsage, memoryUsage);
                if (isMapped)
                    Map();

                VkBufferDeviceAddressInfo addressInfo{};
                addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                addressInfo.buffer = buffer;

                address = vkGetBufferDeviceAddress(aDevice->GetLogicalDevice(), &addressInfo);
            }
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
        VkDeviceAddress address = 0;
        VkDeviceSize bufferSize = 0;
        VkBufferUsageFlags bufferUsage;
        VmaMemoryUsage memoryUsage;
        VmaAllocation allocation;

        int newBufferRecords = 0;

        void replace();
        void cleanup();
    };

    struct CopyBufferInfo
    {
        Buffer* dstBuffer;
        void* src;
        size_t size;
    };

    inline void CopyBuffer(Device* device, uint32_t infoCount, CopyBufferInfo* infos, VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    {
        for (uint32_t i = 0; i < infoCount; i++)
        {
            if (infos[i].dstBuffer->GetSize() < infos[i].size)
            {
                *infos[i].dstBuffer = Buffer(device, infos[i].size, usage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
                infos[i].dstBuffer->Map();
            }
            if (infos[i].src)
                memcpy(infos[i].dstBuffer->GetMappedData(), infos[i].src, infos[i].size);
        }
    }
}
