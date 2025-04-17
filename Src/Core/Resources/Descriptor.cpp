#include "Headers/Descriptor.hpp"

using namespace AnA;

Descriptor::Descriptor(Device* mDevice, Buffer* buffers, VkDeviceSize bufferSize, uint32_t binding,
    uint32_t descriptorSetCount, VkShaderStageFlags stageFlags,
    const VkDescriptorType descriptorType) : aDevice{mDevice}
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    
    VkDescriptorSetLayoutBinding layoutBinding = Device::CreateLayoutBinding(binding, descriptorType, stageFlags);
    layoutInfo.pBindings = &layoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice->GetLogicalDevice(), &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
    layoutCreated = true;

    aDevice->CreateDescriptorPool(descriptorSetCount, pool, descriptorType);
    aDevice->CreateDescriptorSets(buffers, bufferSize, binding, descriptorSetCount, pool, setLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device* mDevice, Buffer* buffers, VkDeviceSize bufferSize, uint32_t binding,
    uint32_t descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout,
    const VkDescriptorType descriptorType) : aDevice{mDevice}
{    
    aDevice->CreateDescriptorPool(descriptorSetCount, pool, descriptorType);
    aDevice->CreateDescriptorSets(buffers, bufferSize, binding, descriptorSetCount, pool, descriptorSetLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device* mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, 
        uint32_t descriptorSetCount, VkShaderStageFlags stageFlags, const VkDescriptorType descriptorType
        ) : aDevice{mDevice}
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    
    VkDescriptorSetLayoutBinding layoutBinding = Device::CreateLayoutBinding(binding, descriptorType, stageFlags);
    layoutInfo.pBindings = &layoutBinding;
    if (vkCreateDescriptorSetLayout(aDevice->GetLogicalDevice(), &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
    layoutCreated = true;

    if (descriptorSetCount == 0)
        return;
    aDevice->CreateDescriptorPool(descriptorSetCount, pool, descriptorType);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    aDevice->CreateDescriptorSets(&imageInfo, binding, descriptorSetCount, pool, setLayout, descriptorType, sets);
}

Descriptor::Descriptor(Device* mDevice, VkSampler& sampler, VkImageView& imageView, VkImageLayout imageLayout, uint32_t binding, 
        uint32_t descriptorSetCount, VkDescriptorSetLayout descriptorSetLayout, const VkDescriptorType descriptorType
        ) : setLayout(descriptorSetLayout)
{
    aDevice = mDevice;
    if (descriptorSetCount == 0)
        return;
    aDevice->CreateDescriptorPool(descriptorSetCount, pool, descriptorType);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    if (descriptorSetCount > 1)
    {
        std::vector<VkDescriptorImageInfo> imageInfos(descriptorSetCount, imageInfo);
        aDevice->CreateDescriptorSets(imageInfos.data(), binding, descriptorSetCount, pool, descriptorSetLayout, descriptorType, sets);
    }
    else
    {
        aDevice->CreateDescriptorSets(&imageInfo, binding, descriptorSetCount, pool, descriptorSetLayout, descriptorType, sets);
    }
}

Descriptor::Descriptor(Device* mDevice, uint32_t descriptorSetCount, uint32_t descriptorCount, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorType descriptorType) : aDevice{mDevice}
{
    aDevice->CreateDescriptorPool(descriptorSetCount, pool, descriptorType, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);
    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{};
    countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    std::vector<uint32_t> counts(descriptorSetCount, descriptorCount);
    countInfo.pDescriptorCounts = counts.data();
    countInfo.descriptorSetCount = descriptorSetCount;
    aDevice->CreateDescriptorSets(descriptorSetCount, pool, descriptorSetLayout, sets, &countInfo);
}

Descriptor::Descriptor(Device* mDevice, Descriptor::DescriptorConfig* descriptorConfigs, uint32_t configCount, uint32_t descriptorSetCount) : aDevice{mDevice}
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    const VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | 
                VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    std::vector<VkDescriptorBindingFlags> bindingFlags{};
    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(configCount);
    std::vector<VkDescriptorPoolSize> poolSizes{};
    std::vector<std::vector<VkWriteDescriptorSet>> writes(descriptorSetCount);
    VkDescriptorPoolCreateFlags poolFlags = 0;
    uint32_t totalDescriptorCount = 0;
    for (uint32_t i = 0; i < configCount; i++)
    {
        auto& descriptorConfig = descriptorConfigs[i];
        VkDescriptorPoolSize poolSize{};

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstBinding = descriptorConfig.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = descriptorConfig.descriptorType;
        descriptorWrite.descriptorCount = descriptorConfig.descriptorCount;
        poolSize.type = descriptorConfig.descriptorType;
        poolSize.descriptorCount = descriptorConfig.descriptorCount;

        auto& layoutBinding = layoutBindings[i];
        layoutBinding = Device::CreateLayoutBinding(descriptorConfig.binding,
        descriptorConfig.descriptorType, descriptorConfig.stageFlags, descriptorConfig.descriptorCount);

        totalDescriptorCount += descriptorConfig.descriptorCount;
        if (descriptorConfig.descriptorCount == 0)
        {
            poolFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            layoutBinding.descriptorCount = MaxBatchSize;
            bindingFlags.push_back(0);

            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
            continue;
        }
        else if (descriptorConfig.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || descriptorConfig.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)
        {
            for (uint32_t i = 0; i < descriptorSetCount; i++)
            {
                descriptorWrite.pImageInfo = &descriptorConfig.imageInfos[i];
                writes[i].push_back(descriptorWrite);
            }
        }
        else
        {
            for (uint32_t i = 0; i < descriptorSetCount; i++)
            {
                descriptorWrite.pBufferInfo = &descriptorConfig.bufferInfos[i];
                writes[i].push_back(descriptorWrite);
            }
        }
        bindingFlags.push_back(0);
        poolSizes.push_back(poolSize);
    }
    if (bindingFlags.size())
    {
        if (poolFlags)
            bindingFlags.back() = bindlessFlags;
        layoutInfo.pNext = &flagsInfo;

        flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
        flagsInfo.pBindingFlags = bindingFlags.data();
    }
    layoutInfo.pBindings = layoutBindings.data();
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());

    if (vkCreateDescriptorSetLayout(aDevice->GetLogicalDevice(), &layoutInfo, nullptr, &setLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the DescriptorSetLayout");
    layoutCreated = true;
    if (!totalDescriptorCount)
        return;
    aDevice->CreateDescriptorPool(descriptorSetCount, poolSizes.data(), static_cast<uint32_t>(poolSizes.size()),
        pool, poolFlags);
    aDevice->CreateDescriptorSets(descriptorSetCount, pool, setLayout, sets, writes);
}

Descriptor::~Descriptor()
{
    auto device = aDevice->GetLogicalDevice();
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
    for (size_t i = 0; i < sets.size(); i++)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = sets[i];
        descriptorWrite.dstBinding = descriptorConfig.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = descriptorConfig.descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &descriptorConfig.imageInfos[i];
        vkUpdateDescriptorSets(aDevice->GetLogicalDevice(), 1,
            &descriptorWrite, 0, nullptr);
    }
}

void Descriptor::UpdateDescriptorSets(VkDescriptorImageInfo* imageInfos, uint32_t imageCount, uint32_t dstBinding, 
    VkDescriptorType descriptorType)
{
    for (size_t i = 0; i < sets.size(); i++)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = sets[i];
        descriptorWrite.dstBinding = dstBinding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = descriptorType;
        descriptorWrite.descriptorCount = (uint32_t)imageCount;
        descriptorWrite.pImageInfo = imageInfos;
        vkUpdateDescriptorSets(aDevice->GetLogicalDevice(), 1,
            &descriptorWrite, 0, nullptr);
    }
}

void Descriptor::UpdateDescriptorSets(std::vector<VkDescriptorImageInfo> imageInfos, uint32_t dstBinding, VkDescriptorType descriptorType)
{
    for (size_t i = 0; i < sets.size(); i++)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = sets[i];
        descriptorWrite.dstBinding = dstBinding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = descriptorType;
        descriptorWrite.descriptorCount = (uint32_t)imageInfos.size();
        descriptorWrite.pImageInfo = imageInfos.data();
        vkUpdateDescriptorSets(aDevice->GetLogicalDevice(), 1,
            &descriptorWrite, 0, nullptr);
    }
}

void Descriptor::UpdateDescriptorSets(VkDescriptorBufferInfo* pBufferInfo, uint32_t dstBinding, VkDescriptorType descriptorType)
{
    for (size_t i = 0; i < sets.size(); i++)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = sets[i];
        descriptorWrite.dstBinding = dstBinding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = pBufferInfo;
        vkUpdateDescriptorSets(aDevice->GetLogicalDevice(), 1,
            &descriptorWrite, 0, nullptr);
    }
}