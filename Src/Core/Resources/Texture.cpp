#include "Headers/Texture.hpp"

#define DEFAULT_FONT_SIZE 32.0f

using namespace AnA;

Texture::Texture(const char* filename, Device* mDevice) : aDevice{ mDevice }
{
    aDevice->CreateTextureImage(filename, &textureImage, &textureImageMemory);
    init();
}

Texture::Texture(const uint32_t color, Device* mDevice) : aDevice{ mDevice }
{
    aDevice->CreateColorImage(color, &textureImage, &textureImageMemory);
    init();
}

Texture::Texture(const char* text, const int width, const int height, const float lineHeight, Device* mDevice) : aDevice{ mDevice }
{
    aDevice->CreateTextImage(text, width, height, lineHeight, &textureImage, &textureImageMemory);
    init();
}

Texture::~Texture()
{
    auto& device = aDevice->GetLogicalDevice();

    vkDestroySampler(device, imageInfo.sampler, nullptr);
    vkDestroyImageView(device, imageInfo.imageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
}

VkImageView& Texture::GetImageView()
{
    return imageInfo.imageView;
}
VkSampler& Texture::GetSampler()
{
    return imageInfo.sampler;
}

VkDescriptorImageInfo& Texture::GetImageInfo()
{
    return imageInfo;
}

Device* Texture::GetDevice()
{
    return aDevice;
}

void Texture::init()
{
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = aDevice->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    createTextureSampler();
    //auto descriptors = Resource::ResourceManager::GetCurrent()->Shaders[0]->GetDescriptors();
    //descriptor = new Descriptor(aDevice, textureSampler, textureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //     0, 1, Resource::ResourceManager::GetCurrent()->Shaders[0]->GetDescriptors()[DEFAULT_SAMPLER_LAYOUT]->GetLayout(),
    //     VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    
}

void Texture::createTextureSampler(enum VkSamplerAddressMode samplerAddressMode)
{
    auto& properties = aDevice->GetPhysicalDeviceProperties();

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

    vkCreateSampler(aDevice->GetLogicalDevice(), &samplerInfo, nullptr, &imageInfo.sampler);
}
