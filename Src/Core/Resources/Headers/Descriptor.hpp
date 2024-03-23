#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../../Headers/Device.hpp"

namespace AnA
{
    class Descriptor
    {
    public:
        struct DescriptorConfig
        {
            VkDescriptorType descriptorType;
            int descriptorCount;
            std::vector<Buffer*> buffers;
            VkDeviceSize bufferSize;
            VkSampler sampler;
            VkImageView imageView;
            VkImageLayout imageLayout;
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