#pragma once

#include <glm/glm.hpp>
#include <array>
#include "../../Headers/Device.hpp"
#include "../../Headers/Buffer.hpp"
#include "Descriptor.hpp"

#define SHADOW_MAP_CASCADE_COUNT 4

namespace AnA
{
    namespace Resource
    {
        struct Cascade
        {
            std::vector<VkImageView> imageViews;
            std::vector<VkFramebuffer> framebuffers;
            float splitDepth;
            glm::mat4 viewProjMatrix;
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
            }
            ~ShadowMap();
            void UpdateBuffers(Cameras::Camera& camera);
        private:
            Device* aDevice{nullptr};
            float cascadeSplitLambda = 0.95f;
            std::vector<VkSampler> shadowSamplers;
            std::vector<Image> shadowImages;
            std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;
            Buffer cascadeBuffer;
            void createShadowResources();
            void cleanupShadowResources();
        };
    }
}