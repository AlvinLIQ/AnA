#pragma once
#include "../../Camera/Headers/Camera.hpp"
#include "../../Headers/Threadpool.hpp"
#include "Animation.hpp"
#include "Meshes.hpp"
#include "Scene.hpp"
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
    struct MiscBufferObject
    {
        uint32_t objectCount;
        uint32_t collidedCount;
        uint32_t meshletCount;
        uint32_t meshletIDCount;
        glm::mat4 viewProj;
        glm::vec4 planes[6];
        glm::vec2 resolution;
        glm::vec3 cameraPosition;
    };
    namespace Resources
    {
        struct DescriptorResources
        {
            VkDescriptorSetLayout setLayout;
            Buffer buffer;
            void cleanup(VkDevice device)
            {
                vkDestroyDescriptorSetLayout(device, setLayout, VK_NULL_HANDLE);
            }
        };
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
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.1f, 1000.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.05f, 32.0f};
            FrustumPlanes MainCameraFrustumPlanes{};
            bool LockCamera = false;
            void UpdateCamera(float aspect);
            void UpdateMiscBuffer();
            void Update();

            void Resize();

            void BindDescriptors(VkCommandBuffer commandBuffer);

            Resources::Meshes Meshes;
            Scene MainScene;
            AnA::Animations Animations;
            //Test scene
            //Scene Points;
            std::vector<Shader> Shaders;

            uint32_t GetSelectedVertexIndex()
            {
                return selectedVertexIndex;
            }
            VkDeviceAddress GetMiscBufferAddress()
            {
                return miscBuffer.GetAddress() + SwapChain::GetCurrent()->CurrentImage * sizeof(MiscBufferObject);
            }
            DescriptorResources& GetSampledImageResources()
            {
                return sampledImageDescriptor;
            }
            AnA::Text TextContext;
            Lights::Light GlobalLight;
            AnA::Shapes Shapes;
#ifdef ANA_INCLUDE_CONTROL
            AnA::Controls::Control* MainControl = NULL;
#endif
            std::unordered_map<uint32_t, Texture> TextureMap{};
            std::unordered_map<std::string, uint32_t> TexturePathMap{};
            std::unordered_map<uint32_t, uint32_t> TextureIdMap{};
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
            uint32_t AppendTexture(const uint32_t color, uint32_t* index = nullptr, const std::string& name = "");
            uint32_t AppendTexture(const std::string& path, uint32_t* index = nullptr);
            uint32_t AppendTexture(VkImage image, VmaAllocation allocation,
                VkImageView imageView, uint32_t* index = nullptr, const std::string& name = "");

            MeshShaderOutput MeshShaderOutputData;
        private:
            Device* aDevice;
            Buffer miscBuffer;
            void createMiscBuffers();

            DescriptorResources sampledImageDescriptor;
            void createSampledImageResources();
            void appendSampledImage(VkDescriptorImageInfo& imageInfo);

            std::vector<NormalCallBack> callbacks{};
            std::mutex callbacksMutex{};
            std::vector<VkDescriptorImageInfo> textureInfos;

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
            std::array<uint32_t, 2> uboDynamicOffsets{0, 0};
        };
    }
}
