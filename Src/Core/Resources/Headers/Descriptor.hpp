#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../../Headers/Device.hpp"

namespace AnA
{
    namespace Resource
    {
        struct Image
        {
            VkImage image;
            VkDeviceMemory imageMemory;
            VkImageView imageView;
            VkImageLayout imageLayout;
            VkImageType imageType;
            VkFormat format;
            VkExtent3D extent;
            void CleanUp(VkDevice& device)
            {
                vkDestroyImageView(device, imageView, nullptr);
                vkDestroyImage(device, image, nullptr);
                vkFreeMemory(device, imageMemory, nullptr);
            }
        };
    }

    class Descriptor
    {
    public:
        struct DescriptorConfig
        {
            VkDescriptorType descriptorType;
            int descriptorCount;
            Buffer** buffers;
            VkDeviceSize bufferSize;
            VkSampler* samplers;
            AnA::Resource::Image* images;
            uint32_t binding;
            VkShaderStageFlags stageFlags;
        };
        Descriptor(Device& mDevice, Buffer** buffers, VkDeviceSize bufferSize, uint32_t binding, int descriptorSetCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);
        Descriptor(Device& mDevice, Buffer** buffers, VkDeviceSize bufferSize, uint32_t binding, int descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout, const VkDescriptorType descriptorType);
        Descriptor(Device& mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, int descriptorSetCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);
        Descriptor(Device& mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, int descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);

        Descriptor(Device& mDevice, Descriptor::DescriptorConfig& descriptorConfig);
        ~Descriptor();

        std::vector<VkDescriptorSet>& GetSets();
        VkDescriptorSetLayout GetLayout();
    private:
        VkDescriptorPool pool;
        VkDescriptorSetLayout setLayout{VK_NULL_HANDLE};
        std::vector<VkDescriptorSet> sets;
        Device& aDevice;
    };
}