#pragma once

#include <glm/glm.hpp>
#include "../../Headers/Device.hpp"
#include "../../Headers/Buffer.hpp"
#include "Descriptor.hpp"

#define SHADOW_MAP_CASCADE_COUNT 2

namespace AnA
{
    namespace Resource
    {
        struct alignas(16) CascadeBufferObject
        {
            glm::mat4 viewProjMatrix;
            float splitDepth;
        };
        struct Cascade
        {
            std::vector<VkImageView> imageViews;
            std::vector<VkFramebuffer> framebuffers;
            void cleanup(VkDevice device)
            {
                for (auto& imageView : imageViews)
                    vkDestroyImageView(device, imageView, nullptr);
                imageViews.clear();
                for (auto& framebuffer : framebuffers)
                    vkDestroyFramebuffer(device, framebuffer, nullptr);
                framebuffers.clear();
            }
        };
        class ShadowMap
        {
        public:
            ShadowMap(Device* mDevice);
            ShadowMap(const ShadowMap&) = delete;
            ShadowMap& operator=(const ShadowMap&) = delete;
            /*
            ShadowMap(ShadowMap&& shadowMap) noexcept : aDevice{shadowMap.aDevice}
            {

            }
            ShadowMap& operator=(ShadowMap&& shadowMap)
            {
                if (&shadowMap != this)
                {
                    ShadowMap::~ShadowMap();
                    aDevice = shadowMap.aDevice;
                    shadowMap.aDevice = nullptr;
                }

                return *this;
            }*/
            ~ShadowMap();
            std::vector<Image>& GetImages()
            {
                return images;
            }
            std::vector<VkSampler>& GetSamplers()
            {
                return samplers;
            }
            std::vector<Buffer>& GetCascadesBuffers()
            {
                return cascadeBuffers;
            }
            std::vector<VkDescriptorImageInfo>& GetDescriptorImageInfos()
            {
                return descriptorImageInfos;
            }
            std::vector<VkFramebuffer>& GetFramebuffers()
            {
                return framebuffers;
            }
            void UpdateBuffers(Cameras::Camera& camera, Cameras::Camera& light, uint32_t bufferIndex);
            void GetUBODescriptorConfig(Descriptor::DescriptorConfig* pConfig);
        private:
            Device* aDevice{nullptr};
            float cascadeSplitLambda = 0.95f;
            std::vector<VkSampler> samplers;
            std::vector<Image> images;
            std::vector<Buffer> cascadeBuffers;
            std::vector<VkDescriptorImageInfo> descriptorImageInfos;
            std::vector<VkFramebuffer> framebuffers;
            void createShadowResources();
            void cleanupShadowResources();
        };
    }
}