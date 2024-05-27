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
        Buffer(Device* mDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~Buffer();
        Buffer(const Buffer&) = delete;
        Buffer &operator=(const Buffer&) = delete;
        Buffer(Buffer&& buf) noexcept
        {
            aDevice = buf.aDevice;
            mappedData = buf.mappedData;
            buffer = buf.buffer;
            bufferSize = buf.bufferSize;
            bufferMemory = buf.bufferMemory;
            bufferUsage = buf.bufferUsage;
            bufferMemoryProperties = buf.bufferMemoryProperties;
            newBufferRecords = buf.newBufferRecords;
            newBuffer = buf.newBuffer;
        }
        Buffer &operator=(Buffer&& buf) noexcept
        {
            if (&buf != this)
            {
                cleanup();
                if (newBuffer)
                    delete newBuffer;
                aDevice = buf.aDevice;
                mappedData = buf.mappedData;
                buffer = buf.buffer;
                bufferSize = buf.bufferSize;
                bufferMemory = buf.bufferMemory;
                bufferUsage = buf.bufferUsage;
                bufferMemoryProperties = buf.bufferMemoryProperties;
                newBufferRecords = buf.newBufferRecords;
                newBuffer = buf.newBuffer;
                buf.mappedData = nullptr;
                buf.buffer = VK_NULL_HANDLE;
                buf.bufferMemory = VK_NULL_HANDLE;
                buf.newBuffer = nullptr;
            }
            return *this;
        }
        VkResult Map(VkDeviceSize offset, VkDeviceSize size);
        void Unmap();

        void UpdateData(void* newData, size_t dataSize);

        VkBuffer& GetBuffer();
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
            return newBuffer == nullptr ? bufferSize : newBuffer->bufferSize;
        }

        void CopyToBuffer(Buffer& srcBuffer, VkDeviceSize bufferSize)
        {
            aDevice->CopyBuffer(srcBuffer.GetBuffer(), buffer, bufferSize);
        }

        void CopyToBuffer(Buffer& srcBuffer, uint32_t regionCount, const VkBufferCopy* regions)
        {
            aDevice->CopyBuffer(srcBuffer.GetBuffer(), buffer, regionCount, regions);
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
        void ReplaceRequest(Buffer* newBuffer);
        static void TryReplace();
    private:
        Device* aDevice = nullptr;

        void* mappedData = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize bufferSize = 0;
        VkBufferUsageFlags bufferUsage;
        VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
        VkMemoryPropertyFlags bufferMemoryProperties;

        int newBufferRecords = 0;

        Buffer* newBuffer{nullptr};

        void replace();
        void cleanup();
    };
}