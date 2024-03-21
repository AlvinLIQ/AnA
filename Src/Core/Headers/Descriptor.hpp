#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Device.hpp"

namespace AnA
{
    class Descriptor
    {
    public:
        Descriptor(Device& mDevice, void** buffers, VkDeviceSize bufferSize, uint32_t binding, int descriptorSetCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);
        Descriptor(Device& mDevice, void** buffers, VkDeviceSize bufferSize, uint32_t binding, int descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout, const VkDescriptorType descriptorType);
        Descriptor(Device& mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, int descriptorSetCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);
        Descriptor(Device& mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, int descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);

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