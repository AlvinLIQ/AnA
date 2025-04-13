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
//#include <map>

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

            static void GetBufferInfos(std::vector<Buffer>& buffers, std::vector<VkDescriptorBufferInfo>& bufferInfos);
            static void GetImageInfos(const std::vector<Image>& images, const std::vector<VkSampler>& samplers, std::vector<VkDescriptorImageInfo>& imageInfos);

            //Built-in resources
            Cameras::Camera MainCamera;
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.05f, 32.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.05f, 32.0f};
            FrustumPlanes MainCameraFrustumPlanes{};
            void UpdateCamera(float aspect);
            void UpdateCameraBuffer();
            void Update();

            void Resize();

            Meshes SceneObjects;
            std::vector<Shader> Shaders;
            void GetDefaultDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);
            void GetDefaultShapesDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);

            Lights::Light GlobalLight;
            AnA::Shapes Shapes;
#ifdef ANA_INCLUDE_CONTROL
            AnA::Controls::Control* MainControl = NULL;
#endif
            std::unordered_map<uint32_t, Texture> TextureMap{};
            std::vector<Character> Characters{};
            AnA::Resource::ShadowMap ShadowMap;
            ThreadPool<void()> TaskPool{MAX_FRAMES_IN_FLIGHT};
            CommandBufferPool SecondaryCommandBufferPool;
            //ThreadPool<void(CommandBuffer*)> SecondaryCommandBufferPool{};
            void RecreateResources();
            std::vector<RecordCallBackInfo> RecordCallBacks{};
        private:
            Device* aDevice;
            std::vector<Buffer> mainCameraBuffers;
            std::vector<Buffer> frustumBuffers;
            void createMainCameraBuffers();

            uint32_t recordedCallbacks = 0;
/*
            std::vector<VkSampler> shadowSamplers;
            std::vector<Image> shadowImages;
            std::vector<VkFramebuffer> shadowFramebuffers;
            void createShadowFramebuffers();
            void cleanupShadowResources();*/
            void createDefaultShaders();
            void initTextures();
        };
    }
}