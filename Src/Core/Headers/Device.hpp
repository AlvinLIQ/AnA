#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <fstream>
#include <vulkan/vulkan.h>

#define INCLUDE_STB_IMAGE

#define MAX_FRAMES_IN_FLIGHT 2

#define numsof(A) sizeof(A) / sizeof(A[0])

#define ANA_TEXT_DEFAULT_LINE_HEIGHT 32
#define IS_ASCII_CHAR(A) (A) <= 127

namespace AnA
{
    inline std::vector<unsigned char> ReadFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open " + filename + "!");
        }
        
        size_t fs = (size_t)file.tellg();
        std::vector<unsigned char> buffer(fs);
        file.seekg(0);
        file.read((char*)buffer.data(), fs);
        file.close();
        return buffer;
    }

    class Device
    {
    public:
        Device(VkInstance &mInstance, VkSurfaceKHR &mSurface);
        ~Device();

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& deviceMemory);
        void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
        void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, uint32_t regionCount, const VkBufferCopy* copyRegions);
        void CopyBufferToImage(VkBuffer& srcBuffer, VkImage& dstImage, VkExtent3D extent);

        void CreateImage(VkImageCreateInfo* pCreateInfo, VkImage* pImage, VkDeviceMemory* pImageMemory);
        VkImageView CreateImageView(VkImage& image, VkFormat format);

        void CreateColorImage(const uint32_t color, VkImage* pTexImage, VkDeviceMemory* pTexMemory);

        #ifdef INCLUDE_STB_IMAGE
        void CreateTextureImage(const char* imagePath, VkImage* pTexImage, VkDeviceMemory* pTexMemory);
        void CreateTextImage(const char* text, int width, int height, float lineHeight, VkImage* pTextImage, VkDeviceMemory* pTextMemory);
        #endif

        void CreateSampler(VkSampler* pSampler, enum VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK);

        void CreateDescriptorPool(int descriptorCount, VkDescriptorPool& descriptorPool, VkDescriptorType descriptorType);
        void CreateDescriptorSets(void** buffers, VkDeviceSize bufferSize, uint32_t binding, int descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorType descriptorType, std::vector<VkDescriptorSet>& descriptorSets);
        void CreateDescriptorSets(VkDescriptorImageInfo* pImageInfo, uint32_t binding, int descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorType descriptorType, std::vector<VkDescriptorSet>& descriptorSets);

        static VkDescriptorSetLayoutBinding CreateLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);

        void TransitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsAndComputeFamily;
            std::optional<uint32_t> presentFamily;
            bool isComplete()
            {
                return graphicsAndComputeFamily.has_value() && presentFamily.has_value();
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
