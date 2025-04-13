#include "Headers/Texture.hpp"

#define DEFAULT_FONT_SIZE 32.0f

using namespace AnA;

Texture::Texture()
{
    
}

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

Texture::Texture(const char* text, int& width, int& height, float lineHeight, Device* mDevice, float scaleX, float scaleY) : aDevice{ mDevice }
{
    aDevice->CreateTextImage(text, width, height, lineHeight, &textureImage, &textureImageMemory, scaleX, scaleY);
    init();
}

Texture::~Texture()
{
    cleanup();
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
    aDevice->CreateSampler(&imageInfo.sampler);
    //auto descriptors = Resource::ResourceManager::GetCurrent()->Shaders[0]->GetDescriptors();
    //descriptor = new Descriptor(aDevice, textureSampler, textureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //     0, 1, Resource::ResourceManager::GetCurrent()->Shaders[0]->GetDescriptors()[DEFAULT_SAMPLER_LAYOUT]->GetLayout(),
    //     VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    
}

void Texture::cleanup()
{
    if (!aDevice)
        return;
    auto& device = aDevice->GetLogicalDevice();

    vkDestroySampler(device, imageInfo.sampler, nullptr);
    vkDestroyImageView(device, imageInfo.imageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
}