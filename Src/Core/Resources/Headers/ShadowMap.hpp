#pragma once

#include <glm/glm.hpp>
#include "../../Headers/Device.hpp"
#include "../../Headers/Buffer.hpp"

#define SHADOW_MAP_CASCADE_COUNT 2

namespace AnA
{
    namespace Resources
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
            std::vector<glm::mat4>& GetCascades()
            {
                return cascades;
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
            glm::vec4 FrustumPlanes[6];
            void UpdateBuffers(Cameras::Camera& camera, Cameras::Camera& light, uint32_t bufferIndex);
        private:
            Device* aDevice{nullptr};
            std::vector<VkSampler> samplers;
            std::vector<Image> images;
            std::vector<Buffer> cascadeBuffers;
            std::vector<glm::mat4> cascades;
            std::vector<VkDescriptorImageInfo> descriptorImageInfos;
            std::vector<VkFramebuffer> framebuffers;
            void createShadowResources();
            void cleanupShadowResources();
        };
    }
}
