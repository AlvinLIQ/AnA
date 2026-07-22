#pragma once

#include <volk.h>
#include <cstdint>
#include <optional>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <glm/glm.hpp>
#include "Utils.hpp"
#include "../ShaderCodes/bindings.h"
#include "../../3rdParty/VulkanMemoryAllocator/include/vk_mem_alloc.h"

#define INCLUDE_STB_IMAGE
//#ifndef RELEASE_BUILD
#define ENABLE_MESH_SHADER
//#endif
#define MAX_FRAMES_IN_FLIGHT 2

#define numsof(A) (sizeof(A) / sizeof(A[0]))

#define ANA_TEXT_DEFAULT_LINE_HEIGHT 18
#define IS_ASCII_CHAR(A) (A) <= 127


#define MaxBatchSize 4096

struct VmaAllocator_T;
typedef VmaAllocator_T* VmaAllocator;
struct VmaAllocation_T;
typedef VmaAllocation_T* VmaAllocation;

namespace AnA
{
    template<typename T>
    inline void ReadFile(const std::string& filename, std::vector<T>& buffer)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open " + filename + "!");
        }

        std::streamsize fs = file.tellg();
        buffer.resize(static_cast<size_t>(fs) / sizeof(T));
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fs);
        file.close();
    }
    inline std::vector<unsigned char> ReadFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open " + filename + "!");
        }

        std::streamsize fs = file.tellg();
        std::vector<unsigned char> buffer(static_cast<size_t>(fs));
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fs);
        file.close();
        return buffer;
    }
    class Buffer;
    class CommandBuffer;
    namespace Cameras
    {
        class Camera;
    }
    class Shader;
    class Texture;
    struct Character
    {
        std::vector<std::vector<glm::vec2>> paths;
        std::vector<glm::vec2> vertices;
        std::vector<uint32_t> indices;
        uint32_t indexOffset;
        glm::vec2 center;
        float width;
        float height;
    };
    namespace Controls
    {
        class Control;
    }
    class Window;

    typedef void(*RecordCallBack)(CommandBuffer& commandBuffer);
    typedef void(*RecordCallBackEx)(CommandBuffer& commandBuffer, size_t index);
    typedef bool(*BoolCallBack)();
    typedef void(*NormalCallBack)();
    typedef void(*ViewportCallBack)(VkOffset2D& offset, VkExtent2D& extent);
    enum RenderPassType {RENDER_PASS_TYPE_ONSCREEN, RENDER_PASS_TYPE_OFFSCREEN, RENDER_PASS_TYPE_SIZE};

    class Device
    {
    public:
        Device(VkInstance &mInstance, VkSurfaceKHR &mSurface);
        ~Device();

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage, VkBuffer& buffer, VmaAllocation& allocation);
        void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);
        void MapBuffer(void** data, VmaAllocation allocation);
        void UnmapBuffer(VmaAllocation);
        void FlushAllocation(VmaAllocation allocation);
        void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage& dstImage, VkExtent3D extent);
        void HostImageLayoutTransition(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CopyHostBufferToImage(void* buffer, VkImage dstImage, VkExtent3D extent);

        void CopyBufferToImage(Buffer* stagingBuffer, VkImage* pTexImage, VkExtent3D& extent);

        void CreateImage(VkImageCreateInfo* pCreateInfo, VkImage* pImage, VmaAllocation& allocation);
        void DestroyImage(VkImage image, VmaAllocation allocation);
        VkImageView CreateImageView(VkImage& image, VkFormat format, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
            VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0,
            1, 0,
            1});

        void CreateColorImage(const uint32_t color, VkImage* pTexImage, VmaAllocation& allocation);

        #ifdef INCLUDE_STB_IMAGE
        void CreateTextureImage(const char* imagePath, VkImage* pTexImage, VmaAllocation& allocation);
        void CreateTextImage(const char* text, int& width, int& height, float lineHeight, VkImage* pTextImage, VmaAllocation& allocation, float scaleX = 1.0f, float scaleY = 1.0f);
        void CreateTextImage(const String& text, int& width, int& height, float lineHeight, VkImage* pTextImage, VmaAllocation& allocation);
        #endif

        void BuildFontVertices(std::unordered_map<int, Character>& characters, int offset = 0, int range = 128);
        static void Triangulation(std::vector<glm::vec2>& vertices, std::vector<glm::uvec2>& edges,
            std::vector<uint32_t>& indices);

        void CreateSampler(VkSampler* pSampler, enum VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS);
        void CreateCommandPool(VkCommandPoolCreateFlags flags, VkCommandPool* pool);

        static VkDescriptorSetLayoutBinding CreateLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
        static std::vector<VkDescriptorSetLayoutBinding> CreateLayoutBindings(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);

        void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);
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
        struct DeviceFeatures
        {
            bool meshShaderSupport;
            bool unifiedLayoutsSupport;
            bool hostImageCopySupport;
            bool bufferDeviceAddressSupport;
        };
        VkDevice GetLogicalDevice();
        VkPhysicalDevice GetPhysicalDevice();
        const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const
        {
            return physicalDeviceProperties;
        }
        const VkPhysicalDeviceDescriptorBufferPropertiesEXT& GetDescriptorBufferProperties() const
        {
            return descriptorBufferProperties;
        }

        bool MeshShaderSupport() const
        {
            return deviceFeatures.meshShaderSupport;
        }
        PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT{ VK_NULL_HANDLE };
        PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT{ VK_NULL_HANDLE };
        PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{ VK_NULL_HANDLE };
        PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR{ VK_NULL_HANDLE };
        PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT{ VK_NULL_HANDLE };

        bool HostImageCopySupport() const
        {
           return deviceFeatures.hostImageCopySupport;
        }
        VmaAllocator GetAllocator();

        VkShaderModule CreateShaderModule(const std::vector<unsigned char>& code);

        static VkImageMemoryBarrier2 ImageMemoryBarrier2(
            VkImage image,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            VkAccessFlags2 srcAccessMask,
            VkAccessFlags2 dstAccessMask,
            VkPipelineStageFlags2 srcStageMask,
            VkPipelineStageFlags2 dstStageMask,
            VkImageAspectFlags aspectMask);
        static void PipelineBarrier2(VkCommandBuffer commandBuffer, VkDependencyFlags dependencyFlags,
            VkImageMemoryBarrier2* imageBarriers, uint32_t imageBarrierCount,
            VkBufferMemoryBarrier2* bufferBarriers, uint32_t bufferBarrierCount);
        static void StageBarrier(VkCommandBuffer commandBuffer,
            VkAccessFlags2 srcAccessMask,
            VkAccessFlags2 dstAccessMask,
            VkPipelineStageFlags2 srcStageMask,
            VkPipelineStageFlags2 dstStageMask);
        static void StageBarrier(VkCommandBuffer commandBuffer,
            VkPipelineStageFlags2 srcStageMask,
            VkPipelineStageFlags2 dstStageMask);
        static void StageBarrier(VkCommandBuffer commandBuffer,
            VkPipelineStageFlags2 StageMask);
        static void ImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image,
            VkImageLayout initLayout, VkImageLayout finalLayout,
            VkAccessFlags srcAccessMask, VkImageAspectFlags aspectMask,
            VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        VkCommandBuffer BeginSubCommands();
        void RecordSingleTimeCommands(void(*recordCallback)(VkCommandBuffer));
        void EndSubCommands();
        void EndSubCommands(std::function<void()> postProcess);
        bool SingleTimeCommandsRecorded();
        bool SingleTimeCommandsSubmitBegan();
        VkCommandBuffer BeginSingleTimeCommandsSubmit();
        void EndSingleTimeCommandsSubmit(VkFence& fence);
    private:
        VkInstance& instance;
        VkSurfaceKHR& surface;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        void pickPhysicalDevice();

        std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
            VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
            VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };
        std::vector<VkSampleCountFlagBits> usableSamples{};
        void checkUsableSamples();

        bool checkDeviceExtensionSupport(VkPhysicalDevice device, DeviceFeatures& _deviceFeatures);
        bool isDeviceSuitable(VkPhysicalDevice device, DeviceFeatures& _deviceFeatures, QueueFamilyIndices& indices);

        DeviceFeatures deviceFeatures{};

        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        QueueFamilyIndices queueFamilyIndices;
        void createLogicalDevice();

        std::mutex subCommandMutex{};
        bool subCommandBufferBegan = false;
        bool subCommandBufferRecorded = false;
        bool subCommandBufferSubmitBegan = false;
        VkCommandPool commandPool{VK_NULL_HANDLE}, subCommandPool{VK_NULL_HANDLE};
        VkCommandBuffer subCommandBuffer{};
        void createSubCommandResources();

        std::vector<Buffer*> stagingBuffers{};
        void cleanupStagingBuffers();

        std::vector<std::function<void()>> subCommandPostProcesses;

        VkPhysicalDeviceProperties physicalDeviceProperties{};
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties{};
        VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties{};

        VmaAllocator allocator{nullptr};
        void createVmaAllocator();
    };

    namespace Resources
    {
        struct Image
        {
            VkImage image;
            VmaAllocation allocation;
            VkImageView imageView;
            VkImageLayout imageLayout;
            VkImageType imageType;
            VkFormat format;
            VkExtent3D extent;
            void create(Device* device, VkImageUsageFlags usage, uint32_t arrayLayers = 1,
                VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0,
                1, 0,
                1})
            {
                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.format = format;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.extent = extent;
                imageInfo.mipLevels = 1;
                imageInfo.arrayLayers = arrayLayers;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo.usage = usage;

                device->CreateImage(&imageInfo, &image, allocation);
                imageView = device->CreateImageView(image, format, VK_IMAGE_VIEW_TYPE_2D, subresourceRange);
            }
            void cleanup(Device* device)
            {
                vkDestroyImageView(device->GetLogicalDevice(), imageView, nullptr);
                device->DestroyImage(image, allocation);
                imageView = VK_NULL_HANDLE;
                image = VK_NULL_HANDLE;
                allocation = {};
            }
            VkDescriptorImageInfo GetDescriptorInfo(VkSampler sampler)
            {
                VkDescriptorImageInfo imageInfo;
                imageInfo.imageLayout = imageLayout;
                imageInfo.imageView = imageView;
                imageInfo.sampler = sampler;

                return imageInfo;
            }
        };
    }
}
