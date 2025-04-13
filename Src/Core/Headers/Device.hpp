#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <fstream>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "Utils.hpp"

#define INCLUDE_STB_IMAGE

#define MAX_FRAMES_IN_FLIGHT 2

#define numsof(A) sizeof(A) / sizeof(A[0])

#define ANA_TEXT_DEFAULT_LINE_HEIGHT 18
#define IS_ASCII_CHAR(A) (A) <= 127

#define DEFAULT_VERTEX_LAYOUT 0
#define DEFAULT_UBO_LAYOUT 1
#define DEFAULT_LIGHT_LAYOUT 2
#define DEFAULT_SAMPLER_LAYOUT 3
#define DEFAULT_SHADOW_SAMPLER_LAYOUT 4
#define DEFAULT_CASCADED_UBO_LAYOUT 5
#define DEFAULT_MESHLET_LAYOUT 6

#define DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT 6

#define MaxBatchSize 2048

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
    class Buffer;
    namespace Cameras
    {
        class Camera;
    }
    class Shader;
    class Texture;
    struct Character
    {
        std::vector<std::vector<glm::vec2>> paths;
        glm::vec2 center;
        float width;
        float height;
    };
    namespace Controls
    {
        class Control;
    }
    class Window;

    typedef void(*RecordCallBack)(VkCommandBuffer commandBuffer);
    typedef void(*RecordCallBackEx)(VkCommandBuffer commandBuffer, size_t index);
    typedef bool(*BoolCallBack)();
    typedef void(*NormalCallBack)();
    typedef void(*ViewportCallBack)(VkOffset2D& offset, VkExtent2D& extent);
    enum RenderPassType {RENDER_PASS_TYPE_ONSCREEN, RENDER_PASS_TYPE_OFFSCREEN, RENDER_PASS_TYPE_SIZE};

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
        VkImageView CreateImageView(VkImage& image, VkFormat format, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, 
            VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 
            1, 0, 
            1});

        void CreateColorImage(const uint32_t color, VkImage* pTexImage, VkDeviceMemory* pTexMemory);

        #ifdef INCLUDE_STB_IMAGE
        void CreateTextureImage(const char* imagePath, VkImage* pTexImage, VkDeviceMemory* pTexMemory);
        void CreateTextImage(const char* text, int& width, int& height, float lineHeight, VkImage* pTextImage, VkDeviceMemory* pTextMemory, float scaleX = 1.0f, float scaleY = 1.0f);
        void CreateTextImage(const String& text, int& width, int& height, float lineHeight, VkImage* pTextImage, VkDeviceMemory* pTextMemory);
        #endif

        void BuildFontVertices(std::vector<Character>& characters, int range = 128);

        void CreateSampler(VkSampler* pSampler, enum VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS);

        void UpdateDescriptorSet(const VkDescriptorBufferInfo& bufferInfo, uint32_t binding, VkDescriptorType descriptorType, VkDescriptorSet descriptorSet);
        void CreateDescriptorPool(int descriptorCount, VkDescriptorPool& descriptorPool, VkDescriptorType descriptorType, VkDescriptorPoolCreateFlags flags = 0);
        void CreateDescriptorPool(uint32_t descriptorSetCount, const VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, VkDescriptorPool& descriptorPool, VkCommandPoolCreateFlags flags);
        void CreateDescriptorSets(Buffer* buffers, VkDeviceSize bufferSize, uint32_t binding, uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorType descriptorType, std::vector<VkDescriptorSet>& descriptorSets);
        void CreateDescriptorSets(VkDescriptorImageInfo* imageInfos, uint32_t binding, uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorType descriptorType, std::vector<VkDescriptorSet>& descriptorSets);
        void CreateDescriptorSets(uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, std::vector<VkDescriptorSet>& descriptorSets, void* pNext);
        void CreateDescriptorSets(uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, std::vector<VkDescriptorSet>& descriptorSets, std::vector<std::vector<VkWriteDescriptorSet>>& writes);

        void CreateCommandPool(VkCommandPoolCreateFlags flags, VkCommandPool* pool);

        static VkDescriptorSetLayoutBinding CreateLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
        static std::vector<VkDescriptorSetLayoutBinding> CreateLayoutBindings(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);

        void TransitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void WaitBufferIdle(VkBuffer &buffer);
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
        VkSampleCountFlagBits GetMaxUsableSampleCount();
        const VkPhysicalDeviceMeshShaderPropertiesEXT& GetMeshShaderProperties() const
        {
            return meshShaderProperties;
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
        const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const
        {
            return physicalDeviceProperties;
        }

        PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT{ VK_NULL_HANDLE };
        PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT{ VK_NULL_HANDLE };
    private:
        VkInstance& instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        void pickPhysicalDevice();

        const std::vector<const char*> deviceExtensions = 
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
        };
        std::vector<VkSampleCountFlagBits> usableSamples{};
        void checkUsableSamples();

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        bool isDeviceSuitable(VkPhysicalDevice device);

        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        void createLogicalDevice();

        VkCommandPool commandPool;

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        VkPhysicalDeviceProperties physicalDeviceProperties{};
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties{};
    };
}
