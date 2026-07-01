#include "Headers/ResourceManager.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/ShaderCodes.hpp"
#include "Headers/Device.hpp"
#include "Resources/Headers/Animation.hpp"

using namespace AnA;
using namespace Resources;

ResourceManager* _resourceManager = nullptr;
uint32_t modelId = 0;
uint32_t texId = 0;

ResourceManager::ResourceManager(Device* mDevice) :
        Meshes(mDevice),
        MainScene(mDevice),
        Animations(),
        //Points(mDevice),
        TextContext(mDevice),
        GlobalLight(mDevice),
        ShadowMap(mDevice)
        /*SecondaryCommandBufferPool(mDevice,
        VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)*/
{
    aDevice = mDevice;
    _resourceManager = this;
    createMiscBuffers();
    //createShadowFramebuffers();
    createDefaultShaders();
    Meshes.Init();
    MainScene.Init();
    if (aDevice->MeshShaderSupport())
        TextContext.Init();
    //Points.Init();
    //Points.Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    Shapes = AnA::Shapes(mDevice);
    //Shapes = new AnA::Shapes(mDevice);
#ifdef ANA_INCLUDE_CONTROL
    Controls::Control::InitControl(SwapChain::GetCurrent());
#endif
    initTextures();
}

ResourceManager::~ResourceManager()
{
    TextureMap.clear();
    //for (auto& shader : Shaders)
    //    delete shader;
    Shaders.clear();
    //delete Shapes;

    //auto logicalDevice = aDevice->GetLogicalDevice();
    //for (auto& shadowSampler : shadowSamplers)
    //   vkDestroySampler(logicalDevice, shadowSampler, nullptr);
    //cleanupShadowResources();

    //delete MainScene;
    //delete GlobalLight;
#ifdef ANA_INCLUDE_CONTROL
    delete MainControl;
#endif
}

ResourceManager* ResourceManager::GetCurrent()
{
    return _resourceManager;
}

void ResourceManager::UpdateCamera(float aspect)
{
    MainCameraInfo.aspect = aspect;
    MainCameraInfo.UpdateCameraPerspective(MainCamera);
    LightCameraInfo.aspect = aspect;
    //LightCameraInfo.UpdateCameraPerspective(LightCamera);
    //const float scale = 10.5f;
    //LightCamera.SetOrthographicProjection(-scale * LightCameraInfo.aspect, -scale, scale * LightCameraInfo.aspect, scale,
    //    -10.0f, 32.0f);
    //LightCamera.SetOrthographicProjection(-scale * LightCameraInfo.aspect, -scale, scale * LightCameraInfo.aspect, scale,
    //    -10.0f, 32.0f);
}

void ResourceManager::UpdateMiscBuffer()
{
    uint32_t bufferIndex = SwapChain::GetCurrent()->CurrentImage;

    //printf("x: %f y: %f\r", curPos.x.value, curPos.y.value);
    //cbo.invView = MainCamera.GetInverseView();
    auto& mbo = reinterpret_cast<MiscBufferObject*>(miscBuffer.GetMappedData())[bufferIndex];
    mbo.objectCount = MainScene.GetMeshCount();
    mbo.meshletCount = MainScene.GetMeshletCount();
    mbo.meshletIDCount = MainScene.GetMeshletIDCount();
    if (!LockCamera)
    {
        FrustumPlanes::ExtractFrustumPlanes(MainCamera.GetProjectionMatrix(), MainCameraFrustumPlanes);
        memcpy(&mbo.planes,
            &MainCameraFrustumPlanes,
            sizeof(FrustumPlanes));
    }
}

void ResourceManager::Update()
{
    if (Meshes.NeedUpdate())
    {
        Meshes.Update();
    }
    if (MainScene.NeedUpdate())
    {
        MainScene.Update();
    }
    if (TextContext.NeedUpdate())
    {
        TextContext.Update();
    }

    float dt = MainCamera.GetSpeedRatio() * 0.5f;
    //Process Ccamera Animation
    if (Animations.ProcessAnimationInfo(MainCamera.AnimationInfo, MainCamera.CameraTransform, dt))
        MainCamera.UpdateViewMatrix();
    UpdateMiscBuffer();

    Animations.Update(MainScene, dt);

    uint32_t frameIndex = SwapChain::GetCurrent()->CurrentFrame;
    GlobalLight.UpdateBuffers(LightCamera, frameIndex);
    ShadowMap.UpdateBuffers(MainCamera, LightCamera, frameIndex);
    if (callbacks.size())
    {
        std::unique_lock<std::mutex> lock(callbacksMutex);
        for (auto& callback : callbacks)
        {
            callback();
        }
        callbacks.clear();
    }
}

void ResourceManager::Resize()
{
    auto extent = SwapChain::GetCurrent()->GetExtent();
    UpdateCamera(static_cast<float>(extent.width) / static_cast<float>(extent.height));
    RecreateResources();

#ifdef DEFERRED
    auto& framebuffers = SwapChain::GetCurrent()->GetOffscreenFramebuffers();
    VkSampler colorSampler = SwapChain::GetCurrent()->GetColorSampler();
    std::vector<std::vector<VkWriteDescriptorSet>> writes(2);
    VkDescriptorImageInfo imageInfos[4 * MAX_FRAMES_IN_FLIGHT];
    auto& sets = lightDescriptors[0].GetSets();
    for (uint32_t i = 0, j; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& framebuffer = framebuffers[i];
        Image* images[4] = {&framebuffer.position, &framebuffer.normal,
            &framebuffer.albedo, &framebuffer.depth};
        writes[i].resize(4);
        for (j = 0; j < 4; j++)
        {
            imageInfos[i * 4 + j]= images[j]->GetDescriptorInfo(colorSampler);
            auto& write = writes[i][j];
            write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &imageInfos[i * 4 + j];
            write.dstSet = sets[i];
            write.dstBinding = j;
        }
    }
    lightDescriptors[0].UpdateDescriptorSets(writes);
#endif
}

void ResourceManager::RecreateResources()
{
    //cleanupShadowResources();
    //createShadowFramebuffers();
}

uint32_t ResourceManager::AppendTexture(const uint32_t color, uint32_t* index, const std::string& name)
{
    auto result = TextureMap.try_emplace(texId, color, aDevice);

    if (index)
        *index = uint32_t(textureInfos.size());
    if (!name.empty())
        TexturePathMap.try_emplace(name, texId);

    TextureIdMap.emplace(result.first->first, uint32_t(textureInfos.size()));

    texId++;
    return result.first->first;
}

uint32_t ResourceManager::AppendTexture(const std::string& path, uint32_t* index)
{
    auto iter = TexturePathMap.find(path);
    if (iter != TexturePathMap.end())
        return iter->second;

    auto result = TextureMap.try_emplace(texId, path.c_str(), aDevice);

    if (index)
        *index = uint32_t(textureInfos.size());
    TexturePathMap.emplace(path, texId);

    TextureIdMap.emplace(result.first->first, uint32_t(textureInfos.size()));

    texId++;
    return result.first->first;
}

uint32_t ResourceManager::AppendTexture(VkImage image, VmaAllocation allocation,
    VkImageView imageView, uint32_t* index, const std::string& name)
{
    auto iter = TexturePathMap.find(name);
    if (iter != TexturePathMap.end())
        return iter->second;

    auto result = TextureMap.try_emplace(texId, image, allocation, imageView, aDevice);

    if (index)
        *index = uint32_t(textureInfos.size());
    if (!name.empty())
        TexturePathMap.try_emplace(name, texId);

    TextureIdMap.emplace(result.first->first, uint32_t(textureInfos.size()));

    texId++;
    return result.first->first;
}

void ResourceManager::createMiscBuffers()
{
    uint32_t imageCount = SwapChain::GetCurrent()->GetImageCount();
    miscBuffer = Buffer(aDevice,
        imageCount * sizeof(MiscBufferObject),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    miscBuffer.Map();
    MainCamera.SetRotateSpeed(float(imageCount) * 1.5f);

}

/*
std::vector<ShaderInfo> basicShaderStageInfos{{Basic_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Basic_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> shapeShaderStageInfos{{Shape_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Shape_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> pointShaderStageInfos{{Point_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Point_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> csmShaderStageInfos{{CascadedShadowMapping_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                  {CascadedShadowMapping_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}}};*/
std::initializer_list<int> shaderConstants{DEFAULT_CULL};
std::vector<ShaderInfo> meshShaderStageInfos{{Mesh_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, shaderConstants},
                                  {Mesh_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, shaderConstants},
                                      {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
/*
std::vector<ShaderInfo> lightShaderStageInfos{{Light_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                {Light_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> textShaderStageInfos{{Text_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                        {Text_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}},
                                            {Text_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> terrainShaderStageInfos{{Terrain_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                        {Terrain_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}},
                                            {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> collisionShaderStageInfos{{CollisionDetect_comp, 0, VK_SHADER_STAGE_COMPUTE_BIT, false, {}}};*/

void ResourceManager::createDefaultShaders()
{
    Shaders.reserve(10);
    Shaders.emplace_back(aDevice, meshShaderStageInfos, VK_NULL_HANDLE, sizeof(MeshPushConstant));
}

const uint32_t defaultTextureColors[] =
{
    0xFFFFFFFFu,
    0xFFCC9999u,
    0xFF99FF99u,
    0xFF9999CCu,
    0x00CC9999u,
};
void ResourceManager::initTextures()
{
    for (auto& color : defaultTextureColors)
    {
        AppendTexture(color);
    }

    aDevice->BuildFontVertices(Characters);
}
