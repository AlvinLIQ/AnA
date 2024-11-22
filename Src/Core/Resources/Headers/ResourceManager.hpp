#pragma once
#include "../../Camera/Headers/Camera.hpp"
#include "../../Headers/Threadpool.hpp"
#include "CommandBufferPool.hpp"
#include "Mesh.hpp"
#include "Descriptor.hpp"
#include "Shader.hpp"
#include "Lights.hpp"
#include "Texture.hpp"
#include "ShadowMap.hpp"

#define ANA_INCLUDE_CONTROL

#ifdef ANA_INCLUDE_CONTROL
#include "../../../GUI/Controls/Headers/Control.hpp"
#endif

#define DEFAULT_TEXTURE_ID 0

namespace AnA
{
    struct RecordCallBackInfo
    {
        BoolCallBack needRecord;
        ViewportCallBack recordCallBack;
        VkOffset2D offset;
        VkExtent2D extent;
    };
    namespace Resource
    {
        class ResourceManager
        {
        public:
            ResourceManager(Device* mDevice);
            ~ResourceManager();

            static ResourceManager* GetCurrent();
            std::vector<Cascade>& GetCascades();

            //Built-in resources
            Cameras::Camera MainCamera;
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.1f, 1000.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.1f, 1000.0f};
            void UpdateCamera(float aspect);
            void UpdateCameraBuffer();
            void Update();

            void Resize();

            Meshes SceneObjects;
            std::vector<Shader> Shaders;
            std::vector<Descriptor::DescriptorConfig> GetDefaultDescriptorConfig();
            std::vector<Descriptor::DescriptorConfig> GetDefaultShapesDescriptorConfig();

            Lights::Light GlobalLight;
            AnA::Shapes Shapes;
#ifdef ANA_INCLUDE_CONTROL
            AnA::Controls::Control* MainControl = NULL;
#endif
            std::unordered_map<uint32_t, Texture> TextureMap;
            ShadowMap ShadowMap;
            ThreadPool<void()> TaskPool{MAX_FRAMES_IN_FLIGHT};
            CommandBufferPool SecondaryCommandBufferPool;
            //ThreadPool<void(CommandBuffer*)> SecondaryCommandBufferPool{};
            void RecreateResources();
            std::vector<RecordCallBackInfo> RecordCallBacks{};
        private:
            Device* aDevice;
            std::vector<Buffer> mainCameraBuffers;
            void createMainCameraBuffers();

/*
            std::vector<VkSampler> shadowSamplers;
            std::vector<Image> shadowImages;
            std::vector<VkFramebuffer> shadowFramebuffers;
            void createShadowFramebuffers();
            void cleanupShadowResources();*/
            void createDefaultShaders();
        };
    }
}