#pragma once
#include "../../Camera/Headers/Camera.hpp"
#include "../../Headers/Threadpool.hpp"
#include "Animation.hpp"
#include "Meshes.hpp"
#include "Scene.hpp"
#include "Descriptor.hpp"
#include "Shader.hpp"
#include "Lights.hpp"
#include "Text.hpp"
#include "Texture.hpp"
#include "ShadowMap.hpp"
#include <mutex>
#include <string>
#include <unordered_map>
//#include <map>

#ifdef CULL
#define DEFAULT_CULL CULL
#else
#define DEFAULT_CULL 3
#endif

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
    struct TerrainPushConstants
    {
        float density;
        float height;
        uint32_t texture;
        uint32_t calNormal;
    };
    namespace Resources
    {
        class ResourceManager
        {
        public:
            ResourceManager(Device* mDevice);
            ~ResourceManager();

            static ResourceManager* GetCurrent();

            static void GetBufferInfos(std::vector<Buffer>& buffers, std::vector<VkDescriptorBufferInfo>& bufferInfos);
            static void GetBufferInfos(Buffer* buffers,
                uint32_t bufferCount,
                std::vector<VkDescriptorBufferInfo>& bufferInfos);
            static void GetBufferInfos(Buffer& buffer,
                uint32_t bufferCount,
                std::vector<VkDescriptorBufferInfo>& bufferInfos);
            static void GetBufferInfos(Buffer& buffer,
                uint32_t bufferCount,
                std::vector<VkDescriptorBufferInfo>& bufferInfos, uint32_t stride);
            static void GetImageInfos(const std::vector<Image>& images, const std::vector<VkSampler>& samplers, std::vector<VkDescriptorImageInfo>& imageInfos);

            //Built-in resources
            Cameras::Camera MainCamera;
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.05f, 1000.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.05f, 32.0f};
            FrustumPlanes MainCameraFrustumPlanes{};
            bool LockCamera = false;
            void UpdateCamera(float aspect);
            void UpdateCameraBuffer();
            void Update();

            void Resize();

            Resources::Meshes Meshes;
            Scene MainScene;
            AnA::Animations Animations;
            //Test scene
            //Scene Points;
            std::vector<Shader> Shaders;
            void GetDefaultDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);
            void GetDefaultShapesDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);
            void GetDefaultTextDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);
            void GetDefaultComputeDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);
            void GetDefaultLightDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs);
            const std::array<uint32_t, 2>& GetDefaultUBODynamicOffsets() const
            {
                return uboDynamicOffsets;
            }

            uint32_t GetSelectedVertexIndex()
            {
                return selectedVertexIndex;
            }
            AnA::Text TextContext;
            Lights::Light GlobalLight;
            AnA::Shapes Shapes;
#ifdef ANA_INCLUDE_CONTROL
            AnA::Controls::Control* MainControl = NULL;
#endif
            std::unordered_map<uint32_t, Texture> TextureMap{};
            std::unordered_map<int, Character> Characters{};
            AnA::Resources::ShadowMap ShadowMap;
            ThreadPool<void()> TaskPool{MAX_FRAMES_IN_FLIGHT};
            //CommandBufferPool SecondaryCommandBufferPool;
            //ThreadPool<void(CommandBuffer*)> SecondaryCommandBufferPool{};
            void RecreateResources();
            void AppendCallback(NormalCallBack callback)
            {
                std::unique_lock<std::mutex> lock(callbacksMutex);
                callbacks.push_back(callback);
            }
            uint32_t AppendTexture(const std::string path);

            std::vector<Descriptor>& GetDefaultDescriptors()
            {
                return defaultDescriptors;
            }

            MeshShaderOutput MeshShaderOutputData;
        private:
            Device* aDevice;
            Buffer mainCameraBuffer;
            Buffer frustumBuffer;
            void createMainCameraBuffers();

            std::vector<NormalCallBack> callbacks{};
            std::mutex callbacksMutex{};
            std::unordered_map<std::string, uint32_t> textureIDMap{};

            uint32_t selectedVertexIndex = 0;
            //uint32_t recordedCallbacks = 0;
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
            std::vector<Descriptor> textDescriptors;
            std::vector<Descriptor> computeDescriptors;
            std::vector<Descriptor> lightDescriptors;
            std::array<uint32_t, 2> uboDynamicOffsets{0, 0};
        };
    }
}
