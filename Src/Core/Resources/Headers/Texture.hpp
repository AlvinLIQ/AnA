#pragma once

#include "../../Headers/Device.hpp"

namespace AnA
{
    class Texture
    {
    public:
        Texture(const char* filename, Device* mDevice);
        Texture(const uint32_t color, Device* mDevice);
        Texture(const char* text, int& width, int& height, float lineHeight, Device* mDevice);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& texture) noexcept : aDevice{texture.aDevice}, textureImage{texture.textureImage}, 
        textureImageMemory{texture.textureImageMemory}, imageInfo{texture.imageInfo}
        {
            texture.textureImage = VK_NULL_HANDLE;
            texture.textureImageMemory = VK_NULL_HANDLE;
            texture.imageInfo.imageView = VK_NULL_HANDLE;
            texture.imageInfo.sampler = VK_NULL_HANDLE;
        }
        Texture& operator=(Texture&& texture) noexcept
        {
            if (this != &texture)
            {
                Texture::~Texture();
                aDevice = texture.aDevice;
                textureImage = texture.textureImage;
                textureImageMemory = texture.textureImageMemory;
                imageInfo.imageView = texture.imageInfo.imageView;
                imageInfo.sampler = texture.imageInfo.sampler;
                texture.textureImage = VK_NULL_HANDLE;
                texture.textureImageMemory = VK_NULL_HANDLE;
                texture.imageInfo.imageView = VK_NULL_HANDLE;
                texture.imageInfo.sampler = VK_NULL_HANDLE;
            }
            return *this;
        }
        ~Texture();

        VkImageView& GetImageView();
        VkSampler& GetSampler();
        VkDescriptorImageInfo& GetImageInfo();

        Device* GetDevice();
    private:
        Device* aDevice{nullptr};

        void init();

        VkImage textureImage{VK_NULL_HANDLE};
        VkDeviceMemory textureImageMemory{VK_NULL_HANDLE};
        VkDescriptorImageInfo imageInfo;
        void createTextureSampler(enum VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    };
}