#include "Headers/ResourceManager.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/ShaderCodes.hpp"
#include "Headers/Device.hpp"
#include "Resources/Headers/Animation.hpp"
#include "../Headers/App.hpp"

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
    createMainCameraBuffers();
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

void ResourceManager::UpdateCameraBuffer()
{
    auto& proj = MainCamera.GetProjectionMatrix();
    auto& view = MainCamera.GetView();
    uint32_t bufferIndex = SwapChain::GetCurrent()->CurrentImage;
    Cameras::CameraBufferObject& cbo =
        static_cast<Cameras::CameraBufferObject*>(mainCameraBuffer.GetMappedData())[bufferIndex];
    cbo.proj = proj;
    cbo.view = view;

    auto& curPos = App::GetCurrent()->GetInputManager().GetCursorPosition();
    auto swapChain = SwapChain::GetCurrent();
    cbo.cursorPosition = {curPos.x.As<float>() * swapChain->ScaleX, curPos.y.As<float>() * swapChain->ScaleY};
    selectedVertexIndex = cbo.selectedIndex;
    //printf("x: %f y: %f\r", curPos.x.value, curPos.y.value);
    mainCameraBuffer.Flush();
    //cbo.invView = MainCamera.GetInverseView();
    uboDynamicOffsets[0] = bufferIndex * sizeof(Cameras::CameraBufferObject);

    auto extent = SwapChain::GetCurrent()->GetExtent();
    cbo.resolution = {static_cast<float>(extent.width), static_cast<float>(extent.height)};
    if (!LockCamera)
    {
        cbo.position = MainCamera.GetInverseView()[3];
        FrustumPlanes::ExtractFrustumPlanes(proj * view, MainCameraFrustumPlanes);
        memcpy(&static_cast<FrustumPlanes*>(frustumBuffer.GetMappedData())[bufferIndex * 2],
            &MainCameraFrustumPlanes,
            sizeof(FrustumPlanes));
    }
    uboDynamicOffsets[1] = bufferIndex * sizeof(FrustumPlanes) * 2;
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
    UpdateCameraBuffer();

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

void ResourceManager::createMainCameraBuffers()
{
    uint32_t imageCount = SwapChain::GetCurrent()->GetImageCount();
    mainCameraBuffer = Buffer(aDevice,
        imageCount * sizeof(Cameras::CameraBufferObject),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    mainCameraBuffer.Map();
    MainCamera.SetRotateSpeed(float(imageCount) * 1.5f);

    frustumBuffer = Buffer(aDevice,
        imageCount * 2 * sizeof(FrustumPlanes),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    frustumBuffer.Map();
}

std::vector<ShaderInfo> basicShaderStageInfos{{Basic_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Basic_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> shapeShaderStageInfos{{Shape_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Shape_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> pointShaderStageInfos{{Point_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Point_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> csmShaderStageInfos{{CascadedShadowMapping_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                  {CascadedShadowMapping_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}}};
std::initializer_list<int> shaderConstants{DEFAULT_CULL};
std::vector<ShaderInfo> meshShaderStageInfos{{Mesh_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, shaderConstants},
                                  {Mesh_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, shaderConstants},
                                      {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> lightShaderStageInfos{{Light_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                {Light_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> textShaderStageInfos{{Text_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                        {Text_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}},
                                            {Text_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> terrainShaderStageInfos{{Terrain_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                        {Terrain_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}},
                                            {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> collisionShaderStageInfos{{CollisionDetect_comp, 0, VK_SHADER_STAGE_COMPUTE_BIT, false, {}}};
void ResourceManager::createDefaultShaders()
{
    Shaders.reserve(10);
    Shaders.emplace_back(aDevice, basicShaderStageInfos, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    Shaders.emplace_back(aDevice, shapeShaderStageInfos, SHAPE_DESCRIPTOR_SET_LAYOUT_COUNT, 0, sizeof(glm::vec2));
    Shaders.emplace_back(aDevice, pointShaderStageInfos, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0, 0, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

    Shaders.emplace_back(aDevice, collisionShaderStageInfos, 2, 0, 0);
    Shaders.emplace_back(aDevice, lightShaderStageInfos, 1, 0, 0);
    if (aDevice->MeshShaderSupport())
    {
        Shaders.emplace_back(aDevice, csmShaderStageInfos, MESH_DESCRIPTOR_SET_LAYOUT_COUNT, 0);
        Shaders.emplace_back(aDevice, meshShaderStageInfos, MESH_DESCRIPTOR_SET_LAYOUT_COUNT, 0, sizeof(uint32_t));
        Shaders.emplace_back(aDevice, textShaderStageInfos, 3, 0, sizeof(glm::vec2));
        Shaders.emplace_back(aDevice, terrainShaderStageInfos, MESH_DESCRIPTOR_SET_LAYOUT_COUNT, 0, sizeof(TerrainPushConstants));
    }
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
