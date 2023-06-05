#pragma once

#include "Device.hpp"

namespace AnA
{
    class Texture
    {
    public:
        Texture(const char* filename, Device& mDevice);
        ~Texture();

        VkImageView& GetImageView();
        VkSampler& GetSampler();

        Device& GetDevice();
        VkDescriptorSet& GetDescriptorSet();
    private:
        Device& aDevice;

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        void createTextureSampler();

        VkDescriptorPool descriptorPool;
        void createDescriptorPool();
        VkDescriptorSet descriptorSet;
        void createDescriptorSet();
    };
}