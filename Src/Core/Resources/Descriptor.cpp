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

    aDevice.CreateDescriptorPool(descriptorSetCount, pool, descriptorType);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    aDevice.CreateDescriptorSets(&imageInfo, binding, descriptorSetCount, pool, setLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device& mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, 
        int descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType
        ) : aDevice(mDevice)
{
    aDevice.CreateDescriptorPool(descriptorSetCount, pool, descriptorType);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    aDevice.CreateDescriptorSets(&imageInfo, binding, descriptorSetCount, pool, descriptorSetLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device& mDevice, Descriptor::DescriptorConfig& descriptorConfig) : aDevice{mDevice}
{
    aDevice.CreateDescriptorPool(descriptorConfig.descriptorCount, pool, descriptorConfig.descriptorType);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;

    VkDescriptorSetLayoutBinding layoutBinding = Device::CreateLayoutBinding(descriptorConfig.binding,
     descriptorConfig.descriptorType, descriptorConfig.stageFlags);
    layoutInfo.pBindings = &layoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice.GetLogicalDevice(), &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");

    if (descriptorConfig.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || descriptorConfig.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = descriptorConfig.imageLayout;
        imageInfo.imageView = descriptorConfig.imageView;
        imageInfo.sampler = descriptorConfig.sampler;
        aDevice.CreateDescriptorSets(&imageInfo, descriptorConfig.binding,
         descriptorConfig.descriptorCount, pool, 
         setLayout, descriptorConfig.descriptorType, sets);
    }
    else
    {
        aDevice.CreateDescriptorSets(descriptorConfig.buffers.data(), descriptorConfig.bufferSize,
         descriptorConfig.binding, descriptorConfig.descriptorCount, pool,
          setLayout, descriptorConfig.descriptorType, sets);
    }
}

Descriptor::~Descriptor()
{
    auto device = aDevice.GetLogicalDevice();
    vkDestroyDescriptorPool(device, pool, nullptr);
    if (setLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
}

std::vector<VkDescriptorSet>& Descriptor::GetSets()
{
    return sets;
}