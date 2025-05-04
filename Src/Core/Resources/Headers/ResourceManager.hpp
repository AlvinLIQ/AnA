#pragma once
#include "../../Camera/Headers/Camera.hpp"
#include "../../Headers/Threadpool.hpp"
#include "CommandBufferPool.hpp"
#include "Scene.hpp"
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
    struct MeshShaderOutput
    {
        uint32_t meshletCount;
        uint32_t vertexCount;
        uint32_t triangleCount;
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
            static void GetBufferInfos(Buffer* buffers, uint32_t bufferSize, std::vector<VkDescriptorBufferInfo>& bufferInfos);
            static void GetImageInfos(const std::vector<Image>& images, const std::vector<VkSampler>& samplers, std::vector<VkDescriptorImageInfo>& imageInfos);

            //Built-in resources
            Cameras::Camera MainCamera;
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.05f, 32.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.05f, 32.0f};
            FrustumPlanes MainCameraFrustumPlanes{};
            bool LockCamera = false;
            void UpdateCamera(float aspect);
            void UpdateCameraBuffer();
            void Update();

            void Resize();

            Scene MainScene;
            //Test scene
            //Scene Points;
            std::vector<Shader> Shaders;
            void GetDefaultDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);
            void GetDefaultShapesDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);

            Lights::Light GlobalLight;
            AnA::Shapes Shapes;
#ifdef ANA_INCLUDE_CONTROL
            AnA::Controls::Control* MainControl = NULL;
#endif
            std::unordered_map<uint32_t, Texture> TextureMap{};
            std::unordered_map<uint32_t, std::shared_ptr<Model>> ModelMap{};
            std::unordered_map<std::string, uint32_t> ModelPathIndexMap{};
            void CreateModel(const char* filePath, uint32_t& id);
            std::vector<Character> Characters{};
            AnA::Resource::ShadowMap ShadowMap;
            ThreadPool<void()> TaskPool{MAX_FRAMES_IN_FLIGHT};
            CommandBufferPool SecondaryCommandBufferPool;
            //ThreadPool<void(CommandBuffer*)> SecondaryCommandBufferPool{};
            void RecreateResources();
            std::vector<RecordCallBackInfo> RecordCallBacks{};
#ifdef ENABLE_MESH_SHADER
            MeshShaderOutput MeshShaderOutputData;
#endif
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
            void createDefaultDescriptors();
            std::vector<Descriptor> defaultDescriptors;
            std::vector<Descriptor> shapeDescriptors;
        };
    }
}
