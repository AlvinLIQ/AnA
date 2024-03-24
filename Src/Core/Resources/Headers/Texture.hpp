#pragma once

#include "../../Headers/Device.hpp"
#include "Descriptor.hpp"

namespace AnA
{
    class Texture
    {
    public:
        Texture(const char* filename, Device& mDevice);
        Texture(const uint32_t color, Device& mDevice);
        Texture(const char* text, const int width, const int height, const float lineHeight, Device& mDevice);
        ~Texture();

        VkImageView& GetImageView();
        VkSampler& GetSampler();

        Device& GetDevice();
        VkDescriptorSet& GetDescriptorSet();
    private:
        Device& aDevice;

        void init();

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        void createTextureSampler(enum VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

        Descriptor* descriptor;
    };
}