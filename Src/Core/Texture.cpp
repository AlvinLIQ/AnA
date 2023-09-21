#include "Headers/Texture.hpp"
#include "RenderSystem/Headers/RenderSystem.hpp"
#include "vulkan/vulkan_core.h"
#include <stdexcept>

using namespace AnA;

Texture::Texture(const char* filename, Device& mDevice) : aDevice { mDevice }
{
    aDevice.CreateTextureImage(filename, &textureImage, &textureImageMemory);
    textureImageView = aDevice.CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    createTextureSampler();

    createDescriptorPool();
    createDescriptorSet();
}

Texture::Texture(const char* text, const int width, const int height, const int lineHeight, Device& mDevice) : aDevice { mDevice }
{
    aDevice.CreateTextImage(text, width, height, &textureImage, &textureImageMemory);
    textureImageView = aDevice.CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    createTextureSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);

    createDescriptorPool();
    createDescriptorSet();
}

Texture::~Texture()
{
    auto& device = aDevice.GetLogicalDevice();
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
}

VkImageView& Texture::GetImageView()
{
    return textureImageView;
}
VkSampler& Texture::GetSampler()
{
    return textureSampler;
}

Device& Texture::GetDevice()
{
    return aDevice;
}

VkDescriptorSet& Texture::GetDescriptorSet()
{
    return descriptorSet;
}

void Texture::createTextureSampler(enum VkSamplerAddressMode samplerAddressMode)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(aDevice.GetPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = samplerAddressMode;
    samplerInfo.addressModeV = samplerAddressMode;
    samplerInfo.addressModeW = samplerAddressMode;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    vkCreateSampler(aDevice.GetLogicalDevice(), &samplerInfo, nullptr, &textureSampler);
}

void Texture::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(aDevice.GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool!");
}

void Texture::createDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &RenderSystems::RenderSystem::GetCurrent()->GetDescriptorSetLayouts()[1];

    if (vkAllocateDescriptorSets(aDevice.GetLogicalDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = GetImageView();
    imageInfo.sampler = GetSampler();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(aDevice.GetLogicalDevice(), 1,
        &descriptorWrite, 0, nullptr);
}