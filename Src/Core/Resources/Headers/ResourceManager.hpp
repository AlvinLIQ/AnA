#pragma once
#include "../../Camera/Headers/Camera.hpp"
#include "Mesh.hpp"
#include "Descriptor.hpp"
#include "Shader.hpp"
#include "Lights.hpp"

//#ifdef ANA_INCLUDE_CONTROL
#include "../../../GUI/Controls/Headers/Control.hpp"
//#endif

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
            std::vector<VkFramebuffer>& GetShadowFramebuffers();

            //Built-in resources
            Cameras::Camera MainCamera;
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.01f, 100.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.01f, 100.0f};
            void UpdateCamera(float aspect);
            void UpdateCameraBuffer();

            Meshes* SceneObjects;
            std::vector<Shader*> Shaders;
            std::vector<Descriptor::DescriptorConfig> GetDefaultDescriptorConfig();

            Lights::Light* GlobalLight;
//#ifdef ANA_INCLUDE_CONTROL
            AnA::Controls::Control* MainControl = NULL;
//#endif

            void UpdateResources();
        private:
            Device& aDevice;
            std::vector<Buffer*> mainCameraBuffers;
            void createMainCameraBuffers();

            std::vector<VkSampler> shadowSamplers;
            std::vector<Image> shadowImages;
            std::vector<VkFramebuffer> shadowFramebuffers;
            void createShadowFramebuffers();
            void cleanupShadowResources();

            void createDefaultShaders();
        };
    }
}