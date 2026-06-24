#pragma once
#include <volk.h>
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
            uint32_t descriptorCount;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> imageInfos;
            uint32_t binding;
            VkShaderStageFlags stageFlags;
            bool bindless;
        };
        static void CreateDescriptors(Device* aDevice, std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs, std::vector<Descriptor>& descriptors)
        {
            descriptors.reserve(descriptorSetConfigs.size());
            for (auto& descriptorSetConfig : descriptorSetConfigs)
            {
                descriptors.emplace_back(aDevice, descriptorSetConfig.data(),
                static_cast<uint32_t>(descriptorSetConfig.size()), MAX_FRAMES_IN_FLIGHT);
            }
        }
        Descriptor(Device* mDevice, Buffer* buffers, VkDeviceSize bufferSize, uint32_t binding, uint32_t descriptorSetCount, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);
        Descriptor(Device* mDevice, Buffer* buffers, VkDeviceSize bufferSize, uint32_t binding, uint32_t descriptorSetCount, uint32_t descriptorCount, VkDescriptorSetLayout descriptorSetLayout, const VkDescriptorType descriptorType);
        Descriptor(Device* mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, uint32_t descriptorSetCount, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType);
        Descriptor(Device* mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, uint32_t descriptorSetCount, uint32_t descriptorCount, VkDescriptorSetLayout descriptorSetLayout, const VkDescriptorType descriptorType);
        Descriptor(Device* mDevice, uint32_t descriptorSetCount, uint32_t descriptorCount, uint32_t bindingCount, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorType descriptorType);

        Descriptor(Device* mDevice, Descriptor::DescriptorConfig* descriptorConfigs, uint32_t configCount, uint32_t descriptorSetCount);
        ~Descriptor();

        std::vector<VkDescriptorSet>& GetSets();
        const VkDescriptorSetLayout& GetLayout() const;
        void UpdateDescriptorSets(DescriptorConfig& descriptorConfig);
        void UpdateDescriptorSets(VkDescriptorImageInfo* imageInfos, uint32_t imageCount, uint32_t dstBinding = 0, VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        void UpdateDescriptorSets(std::vector<VkDescriptorImageInfo> imageInfos, uint32_t dstBinding = 0, VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        void UpdateDescriptorSets(VkDescriptorBufferInfo* pBufferInfo, uint32_t dstBinding, VkDescriptorType descriptorType);
        void UpdateDescriptorSets(std::vector<std::vector<VkWriteDescriptorSet>>& writes);
    private:
        VkDescriptorPool pool{VK_NULL_HANDLE};
        bool layoutCreated = false;
        VkDescriptorSetLayout setLayout{VK_NULL_HANDLE};
        std::vector<VkDescriptorSet> sets;
        Device* aDevice;
    };
}
