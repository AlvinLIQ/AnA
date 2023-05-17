#pragma once

#include <cstdint>
#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace AnA
{
    class Device
    {
    public:
        Device(VkInstance &mInstance, VkSurfaceKHR &mSurface);
        ~Device();

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &deviceMemory);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;
            bool isComplete()
            {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        QueueFamilyIndices GetQueueFamiliesForCurrent();

        VkQueue &GetGraphicsQueue();
        VkQueue &GetPresentQueue();

        VkCommandPool &GetCommandPool()
        {
            return commandPool;
        }
        
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        struct SwapChainSupportDetails 
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

        VkDevice &GetLogicalDevice();
        VkPhysicalDevice &GetPhysicalDevice();
    private:
        VkInstance& instance;
        VkSurfaceKHR& surface;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        void pickPhysicalDevice();

        const std::vector<const char*> deviceExtensions = 
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        bool isDeviceSuitable(VkPhysicalDevice device);

        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        void createLogicalDevice();

        VkCommandPool commandPool;
        void createCommandPool();

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    };
}
