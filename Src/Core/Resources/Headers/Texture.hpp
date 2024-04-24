#pragma once

#include "../../Headers/Device.hpp"

namespace AnA
{
    class Texture
    {
    public:
        Texture(const char* filename, Device* mDevice);
        Texture(const uint32_t color, Device* mDevice);
        Texture(const char* text, const int width, const int height, const float lineHeight, Device* mDevice);
        ~Texture();

        VkImageView& GetImageView();
        VkSampler& GetSampler();
        VkDescriptorImageInfo& GetImageInfo();

        Device* GetDevice();
    private:
        Device* aDevice;

        void init();

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkDescriptorImageInfo imageInfo;
        void createTextureSampler(enum VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    };
}