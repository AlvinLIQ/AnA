#pragma once
#include "../../Camera/Headers/Camera.hpp"
#include "Object.hpp"
#include "Descriptor.hpp"
#include "Shader.hpp"
#include "Lights.hpp"

namespace AnA
{
    namespace Resource
    {
        class ResourceManager
        {
        public:
            ResourceManager(Device& mDevice);
            ~ResourceManager();

            static ResourceManager* GetCurrent();

            //Built-in resources
            Cameras::Camera MainCamera;
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.01f, 100.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.5f, 96.0f};
            void UpdateCamera(float aspect);
            void UpdateCameraBuffer();

            Objects* SceneObjects;
            std::vector<Shader*> Shaders;
            std::vector<Descriptor::DescriptorConfig> GetDefault();

            Lights::Light* GlobalLight;
        private:
            Device& aDevice;
            std::vector<Buffer*> mainCameraBuffers;
            void createMainCameraBuffers();

            VkSampler textureSampler;
            void createTextureSampler();
            VkSampler shadowSampler;
            void createShadowSampler();
            std::vector<Image> shadowImages;
            std::vector<VkFramebuffer> shadowFramebuffers;
            void createShadowFramebuffers();
            void cleanupShadowResources();

            void createDefaultShaders();
        };
    }
}