#include "Headers/Texture.hpp"

using namespace AnA;

Texture::Texture(const char* filename, Device& mDevice) : aDevice { mDevice }
{
    aDevice.CreateTextureImage(filename, &textureImage, &textureImageMemory);
    textureImageView = aDevice.CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    createTextureSampler();
}

Texture::~Texture()
{
    auto& device = aDevice.GetLogicalDevice();
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

void Texture::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(aDevice.GetPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    vkCreateSampler(aDevice.GetLogicalDevice(), &samplerInfo, nullptr, &textureSampler);
}