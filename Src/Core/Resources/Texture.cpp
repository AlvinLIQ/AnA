#include "Headers/Texture.hpp"

#define DEFAULT_FONT_SIZE 32.0f

using namespace AnA;

Texture::Texture()
{

}

Texture::Texture(const char* filename, Device* mDevice) : aDevice{ mDevice }
{
    aDevice->CreateTextureImage(filename, &textureImage, allocation);
    init();
}

Texture::Texture(const uint32_t color, Device* mDevice) : aDevice{ mDevice }
{
    aDevice->CreateColorImage(color, &textureImage, allocation);
    init();
}

Texture::Texture(const char* text, int& width, int& height, float lineHeight, Device* mDevice, float scaleX, float scaleY) : aDevice{ mDevice }
{
    aDevice->CreateTextImage(text, width, height, lineHeight, &textureImage, allocation, scaleX, scaleY);
    init();
}

Texture::Texture(VkImage _image, VmaAllocation _allocation, Device* mDevice) : aDevice{mDevice}
{
    textureImage = _image;
    allocation = _allocation;
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

VkImageDescriptorInfoEXT& Texture::GetImageHeapInfo()
{
    return imageHeapInfo;
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
    imageViewInfo = aDevice->ImageViewInfo(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    if (aDevice->DescriptorHeapSupport())
    {
        imageHeapInfo.sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT;
        imageHeapInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageHeapInfo.pView = &imageViewInfo;
    }
    else
    {
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCreateImageView(aDevice->GetLogicalDevice(), &imageViewInfo, VK_NULL_HANDLE, &imageInfo.imageView);
    }

    //imageInfo.sampler = SwapChain::GetCurrent()->GetColorSampler();
    //auto descriptors = Resources::ResourceManager::GetCurrent()->Shaders[0]->GetDescriptors();
    //descriptor = new Descriptor(aDevice, textureSampler, textureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //     0, 1, Resources::ResourceManager::GetCurrent()->Shaders[0]->GetDescriptors()[DEFAULT_SAMPLER_LAYOUT]->GetLayout(),
    //     VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

}

void Texture::cleanup()
{
    if (!aDevice)
        return;
    auto device = aDevice->GetLogicalDevice();

    if (imageInfo.imageView)
        vkDestroyImageView(device, imageInfo.imageView, nullptr);

    aDevice->DestroyImage(textureImage, allocation);
}
