#include "Headers/Descriptor.hpp"

using namespace AnA;

Descriptor::Descriptor(Device& mDevice, Buffer** buffers, VkDeviceSize bufferSize, uint32_t binding,
    int descriptorSetCount, VkShaderStageFlags stageFlags,
    const VkDescriptorType descriptorType) : aDevice(mDevice)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    
    VkDescriptorSetLayoutBinding layoutBinding = Device::CreateLayoutBinding(binding, descriptorType, stageFlags);
    layoutInfo.pBindings = &layoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
    layoutCreated = true;

    aDevice.CreateDescriptorPool(descriptorSetCount, pool, descriptorType);
    aDevice.CreateDescriptorSets(buffers, bufferSize, binding, descriptorSetCount, pool, setLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device& mDevice, Buffer** buffers, VkDeviceSize bufferSize, uint32_t binding,
    int descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout,
    const VkDescriptorType descriptorType) : aDevice(mDevice)
{    
    aDevice.CreateDescriptorPool(descriptorSetCount, pool, descriptorType);
    aDevice.CreateDescriptorSets(buffers, bufferSize, binding, descriptorSetCount, pool, descriptorSetLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device& mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, 
        int descriptorSetCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType
        ) : aDevice(mDevice)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    
    VkDescriptorSetLayoutBinding layoutBinding = Device::CreateLayoutBinding(binding, descriptorType, stageFlags);
    layoutInfo.pBindings = &layoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
    layoutCreated = true;

    if (descriptorSetCount == 0)
        return;
    aDevice.CreateDescriptorPool(descriptorSetCount, pool, descriptorType);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    aDevice.CreateDescriptorSets(&imageInfo, binding, descriptorSetCount, pool, setLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device& mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, 
        int descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType
        ) : aDevice(mDevice), setLayout(descriptorSetLayout)
{
    if (descriptorSetCount == 0)
        return;
    aDevice.CreateDescriptorPool(descriptorSetCount, pool, descriptorType);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    if (descriptorSetCount > 1)
    {
        std::vector<VkDescriptorImageInfo> imageInfos(descriptorSetCount, imageInfo);
        aDevice.CreateDescriptorSets(imageInfos.data(), binding, descriptorSetCount, pool, descriptorSetLayout, descriptorType, sets);
    }
    else
    {
        aDevice.CreateDescriptorSets(&imageInfo, binding, descriptorSetCount, pool, descriptorSetLayout, descriptorType, sets);
    }
}

Descriptor::Descriptor(Device& mDevice, Descriptor::DescriptorConfig& descriptorConfig) : aDevice{mDevice}
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;

    VkDescriptorSetLayoutBinding layoutBinding = Device::CreateLayoutBinding(descriptorConfig.binding,
     descriptorConfig.descriptorType, descriptorConfig.stageFlags);
    layoutInfo.pBindings = &layoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
    layoutCreated = true;

    if (descriptorConfig.descriptorCount == 0)
        return;
    aDevice.CreateDescriptorPool(descriptorConfig.descriptorCount, pool, descriptorConfig.descriptorType);
    if (descriptorConfig.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || descriptorConfig.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)
    {
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.resize(descriptorConfig.descriptorCount);
        for (int i = 0; i < descriptorConfig.descriptorCount; i++)
        {
            
            imageInfos[i].imageLayout = descriptorConfig.images[i].imageLayout;
            imageInfos[i].imageView = descriptorConfig.images[i].imageView;
            imageInfos[i].sampler = descriptorConfig.samplers[i];
        }
        aDevice.CreateDescriptorSets(imageInfos.data(), descriptorConfig.binding,
            descriptorConfig.descriptorCount, pool, 
            setLayout, descriptorConfig.descriptorType, sets);
    }
    else
    {
        aDevice.CreateDescriptorSets(descriptorConfig.buffers, descriptorConfig.bufferSize,
         descriptorConfig.binding, descriptorConfig.descriptorCount, pool,
          setLayout, descriptorConfig.descriptorType, sets);
    }
}

Descriptor::~Descriptor()
{
    auto device = aDevice.GetLogicalDevice();
    if (pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, pool, nullptr);
    if (layoutCreated)
        vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
}

std::vector<VkDescriptorSet>& Descriptor::GetSets()
{
    return sets;
}

const VkDescriptorSetLayout& Descriptor::GetLayout() const
{
    return setLayout;
}

void Descriptor::UpdateDescriptorSets(DescriptorConfig& descriptorConfig)
{
    for (int i = 0; i < sets.size(); i++)
    {
        VkDescriptorImageInfo imageInfo;
        imageInfo.imageLayout = descriptorConfig.images[i].imageLayout;
        imageInfo.imageView = descriptorConfig.images[i].imageView;
        imageInfo.sampler = descriptorConfig.samplers[i];

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = sets[i];
        descriptorWrite.dstBinding = descriptorConfig.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = descriptorConfig.descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(aDevice.GetLogicalDevice(), 1,
            &descriptorWrite, 0, nullptr);
    }
}