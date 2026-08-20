#pragma once

#include "../../Headers/Device.hpp"

namespace AnA
{
    class Texture
    {
    public:
        Texture();
        Texture(const char* filename, Device* mDevice);
        Texture(const uint32_t color, Device* mDevice);
        Texture(const char* text, int& width, int& height, float lineHeight, Device* mDevice, float scaleX = 1.0f, float scaleY = 1.0f);
        Texture(VkImage _image, VmaAllocation _allocation, Device* mDevice);
        Texture(VkImage _image, VmaAllocation _allocation, VkImageView imageView, Device* mDevice);
        Texture(Texture& texture) noexcept
        {
            cleanup();
            aDevice = texture.aDevice;
            textureImage = texture.textureImage;
            allocation = texture.allocation;
            imageInfo = texture.imageInfo;
            texture.textureImage = VK_NULL_HANDLE;
            texture.allocation = VK_NULL_HANDLE;
            texture.imageInfo.imageView = VK_NULL_HANDLE;
            texture.imageInfo.sampler = VK_NULL_HANDLE;
        }
        Texture& operator=(Texture& texture) noexcept
        {
            if (this != &texture)
            {
                cleanup();
                aDevice = texture.aDevice;
                textureImage = texture.textureImage;
                allocation = texture.allocation;
                imageInfo.imageView = texture.imageInfo.imageView;
                imageInfo.sampler = texture.imageInfo.sampler;
                texture.textureImage = VK_NULL_HANDLE;
                texture.allocation = VK_NULL_HANDLE;
                texture.imageInfo.imageView = VK_NULL_HANDLE;
                texture.imageInfo.sampler = VK_NULL_HANDLE;
            }
            return *this;
        }
        ~Texture();

        VkImageView& GetImageView();
        VkSampler& GetSampler();
        VkImageDescriptorInfoEXT& GetImageHeapInfo();
        VkDescriptorImageInfo& GetImageInfo();

        Device* GetDevice();
    private:
        Device* aDevice{nullptr};

        void init();
        void cleanup();

        VkImage textureImage{VK_NULL_HANDLE};
        VmaAllocation allocation{VK_NULL_HANDLE};
        VkImageDescriptorInfoEXT imageHeapInfo{};
        VkDescriptorImageInfo imageInfo{};
    };
}
