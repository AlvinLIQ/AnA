#include "Headers/ResourceManager.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/ShaderCodes.hpp"

using namespace AnA;
using namespace Resource;

ResourceManager* _resourceManager = nullptr;
uint32_t modelId = 0;

ResourceManager::ResourceManager(Device* mDevice) :
        MainScene(mDevice),
        //Points(mDevice),
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
    createDefaultDescriptors();
    createDefaultShaders();
    MainScene.Init();
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

void ResourceManager::GetBufferInfos(std::vector<Buffer>& buffers, std::vector<VkDescriptorBufferInfo>& bufferInfos)
{
    bufferInfos.resize(buffers.size());
    for (size_t i = 0; i < bufferInfos.size(); i++)
    {
        auto& bufferInfo = bufferInfos[i];
        bufferInfo.buffer = buffers[i].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = buffers[i].GetSize();
    }
}

void ResourceManager::GetBufferInfos(Buffer* buffers, uint32_t bufferSize, std::vector<VkDescriptorBufferInfo>& bufferInfos)
{
    bufferInfos.resize(bufferSize);
    for (size_t i = 0; i < bufferInfos.size(); i++)
    {
        auto& bufferInfo = bufferInfos[i];
        bufferInfo.buffer = buffers[i].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = buffers[i].GetSize();
    }
}

void ResourceManager::GetImageInfos(const std::vector<Image>& images, const std::vector<VkSampler>& samplers, std::vector<VkDescriptorImageInfo>& imageInfos)
{
    imageInfos.resize(images.size());
    for (size_t i = 0; i < imageInfos.size(); i++)
    {
        auto& imageInfo = imageInfos[i];
        imageInfo.imageLayout = images[i].imageLayout;
        imageInfo.imageView = images[i].imageView;
        imageInfo.sampler = samplers[i];
    }
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
    uint32_t currentFrame = SwapChain::GetCurrent()->CurrentFrame;
    Cameras::CameraBufferObject& cbo = *static_cast<Cameras::CameraBufferObject*>(mainCameraBuffers[currentFrame].GetMappedData());
    cbo.proj = proj;
    cbo.view = view;
    //cbo.invView = MainCamera.GetInverseView();

    auto extent = SwapChain::GetCurrent()->GetExtent();
    cbo.resolution = {static_cast<float>(extent.width), static_cast<float>(extent.height)};
    if (!LockCamera)
    {
        cbo.position = MainCamera.GetInverseView()[3];
        FrustumPlanes::ExtractFrustumPlanes(proj * view, MainCameraFrustumPlanes);
        memcpy(frustumBuffers[currentFrame].GetMappedData(), &MainCameraFrustumPlanes, sizeof(FrustumPlanes));
    }
}

void ResourceManager::Update()
{
    UpdateCameraBuffer();
    uint32_t frameIndex = SwapChain::GetCurrent()->CurrentFrame;
    GlobalLight.UpdateBuffers(LightCamera, frameIndex);
    ShadowMap.UpdateBuffers(MainCamera, LightCamera, frameIndex);
    if (MainScene.NeedUpdate())
    {
        MainScene.CommitBufferUpdate();
    }
    /*
    recordedCallbacks = 0;
    for (auto& recordCallBackInfo : RecordCallBacks)
    {
        if (recordCallBackInfo.needRecord())
        {
            if (!recordedCallbacks)
                SecondaryCommandBufferPool.Reset();

            recordCallBackInfo.recordCallBack(recordCallBackInfo.offset, recordCallBackInfo.extent);
            ++recordedCallbacks;
        }
    }*/
}

void ResourceManager::Resize()
{
    auto extent = SwapChain::GetCurrent()->GetExtent();
    UpdateCamera(static_cast<float>(extent.width) / static_cast<float>(extent.height));
    RecreateResources();
}

void ResourceManager::RecreateResources()
{
    //cleanupShadowResources();
    //createShadowFramebuffers();
}

void ResourceManager::GetDefaultDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs)
{
    descriptorSetConfigs.resize(DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT);
    for (auto& configs : descriptorSetConfigs)
        configs.resize(1);
    descriptorSetConfigs[DEFAULT_VERTEX_LAYOUT].resize(2);
    auto pConfig = &descriptorSetConfigs[DEFAULT_VERTEX_LAYOUT][0];
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    if (aDevice->MeshShaderSupport())
        pConfig->stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;

    pConfig = &descriptorSetConfigs[DEFAULT_VERTEX_LAYOUT][1];
    pConfig->binding = 1;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    if (aDevice->MeshShaderSupport())
        pConfig->stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;

    pConfig = &descriptorSetConfigs[DEFAULT_UBO_LAYOUT][0];
    pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT;
    GetBufferInfos(mainCameraBuffers, pConfig->bufferInfos);
    descriptorSetConfigs[DEFAULT_UBO_LAYOUT].resize(2);
    pConfig = &descriptorSetConfigs[DEFAULT_UBO_LAYOUT][1];
    pConfig->binding = 1;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT;
    GetBufferInfos(frustumBuffers, pConfig->bufferInfos);

    pConfig = &descriptorSetConfigs[DEFAULT_LIGHT_LAYOUT][0];
    pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT;
    GetBufferInfos(GlobalLight.GetBuffers(), pConfig->bufferInfos);

    pConfig = &descriptorSetConfigs[DEFAULT_SAMPLER_LAYOUT][0];
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pConfig->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pConfig = &descriptorSetConfigs[DEFAULT_SHADOW_SAMPLER_LAYOUT][0];
    pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pConfig->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pConfig->imageInfos = ShadowMap.GetDescriptorImageInfos();
    //GetImageInfos(ShadowMap.GetImages(), ShadowMap.GetSamplers(), pConfig->imageInfos);

    ShadowMap.GetUBODescriptorConfig(&descriptorSetConfigs[DEFAULT_CASCADED_UBO_LAYOUT][0]);
    if (aDevice->MeshShaderSupport())
    {
        Descriptor::DescriptorConfig meshletConfig{};
        meshletConfig.binding = 0;
        meshletConfig.descriptorCount = 0;
        meshletConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
        Descriptor::DescriptorConfig meshletVertexConfig{};
        meshletVertexConfig.binding = 1;
        meshletVertexConfig.descriptorCount = 0;
        meshletVertexConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletVertexConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
        Descriptor::DescriptorConfig meshletIndexConfig{};
        meshletIndexConfig.binding = 2;
        meshletIndexConfig.descriptorCount = 0;
        meshletIndexConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletIndexConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
        Descriptor::DescriptorConfig meshletCullingConfig{};
        meshletCullingConfig.binding = 3;
        meshletCullingConfig.descriptorCount = 0;
        meshletCullingConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletCullingConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT;
        Descriptor::DescriptorConfig meshletIDConfig{};
        meshletIDConfig.binding = 4;
        meshletIDConfig.descriptorCount = 0;
        meshletIDConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletIDConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
        Descriptor::DescriptorConfig meshletIDCountConfig{};
        meshletIDCountConfig.binding = 5;
        meshletIDCountConfig.descriptorCount = 0;
        meshletIDCountConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletIDCountConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;

        descriptorSetConfigs.push_back({meshletConfig, meshletVertexConfig, meshletIndexConfig, meshletCullingConfig, meshletIDConfig, meshletIDCountConfig});
    }
}

void ResourceManager::GetDefaultShapesDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs)
{
    descriptorSetConfigs.resize(2);
    for (auto& configs : descriptorSetConfigs)
        configs.resize(1);
    auto pConfig = descriptorSetConfigs[0].begin();
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pConfig = descriptorSetConfigs[1].begin();
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pConfig->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
}

bool ResourceManager::CreateModel(const char* filePath, uint32_t& id)
{
    auto iter = ModelPathIndexMap.find(filePath);
    if (iter != ModelPathIndexMap.end())
    {
        id = iter->second;
        return false;
    }
    std::shared_ptr<Model> model;
    Model::CreateModelFromFile(filePath, model);
    ModelPathIndexMap.emplace(filePath, modelId);
    id = modelId;
    ModelMap.emplace(modelId++, model);

    return true;
}

void ResourceManager::AppendModel(std::shared_ptr<Model> model, uint32_t& id)
{
    id = modelId;
    ModelMap.emplace(modelId++, model);
}

void ResourceManager::createMainCameraBuffers()
{
    VkDeviceSize bufferSize = sizeof(Cameras::CameraBufferObject);
    mainCameraBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto &cameraBuffer : mainCameraBuffers)
    {
        cameraBuffer = Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        cameraBuffer.Map();
    }

    frustumBuffers.resize(mainCameraBuffers.size());
    for (auto& frustumBuffer : frustumBuffers)
    {
        frustumBuffer = Buffer(aDevice, sizeof(FrustumPlanes) + sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frustumBuffer.Map();
    }
}

void ResourceManager::createDefaultDescriptors()
{
    std::vector<std::vector<Descriptor::DescriptorConfig>> descriptorSetConfigs;
    GetDefaultDescriptorSetConfig(descriptorSetConfigs);
    Descriptor::CreateDescriptors(aDevice, descriptorSetConfigs, defaultDescriptors);

    std::vector<std::vector<Descriptor::DescriptorConfig>> shapesDescriptorConfig;
    GetDefaultShapesDescriptorSetConfig(shapesDescriptorConfig);

    Descriptor::CreateDescriptors(aDevice, shapesDescriptorConfig, shapeDescriptors);
}

void ResourceManager::createDefaultShaders()
{
    Shaders.reserve(6);
    auto renderPass = SwapChain::GetCurrent()->GetRenderPass();
    Shaders.emplace_back(aDevice, Basic_vert, Mesh_frag, renderPass, defaultDescriptors, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    Shaders.emplace_back(aDevice, Shape_vert, Shape_frag, renderPass, shapeDescriptors, SHAPE_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    auto offscreenRenderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();

    Shaders.emplace_back(aDevice, CascadedShadowMapping_task, CascadedShadowMapping_mesh, 
        defaultDescriptors, MESH_DESCRIPTOR_SET_LAYOUT_COUNT, 0, offscreenRenderPass, 0);
    Shaders.emplace_back(aDevice, Point_vert, Point_frag, renderPass, defaultDescriptors, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    if (aDevice->MeshShaderSupport())
    {
        Shaders.emplace_back(aDevice, Terrain_task, Terrain_mesh, Mesh_frag, renderPass, defaultDescriptors, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0, sizeof(TerrainPushConstants));
        Shaders.emplace_back(aDevice, Mesh_task, Mesh_mesh, Mesh_frag, renderPass
            , defaultDescriptors, MESH_DESCRIPTOR_SET_LAYOUT_COUNT, 0, sizeof(uint32_t));
    }
}

void ResourceManager::initTextures()
{
    TextureMap.try_emplace(DEFAULT_TEXTURE_ID, 0xFFFFFFFFu, aDevice);
    TextureMap.try_emplace(1, 0xFFCC9999u, aDevice);
    TextureMap.try_emplace(2, 0xFF99CC99u, aDevice);
    TextureMap.try_emplace(3, 0xFF9999CCu, aDevice);
    TextureMap.try_emplace(4, "Textures/rock3.jpg", aDevice);

    aDevice->BuildFontVertices(Characters);
/*
    char chStr[2];
    chStr[1] = '\0';
    uint32_t width = 0, height = 0;
    for (unsigned char i = 1; i < 128; i++)
    {
        chStr[0] = (char)i;
        CharacterMap.try_emplace(static_cast<char>(i), chStr, width, height, 0, aDevice, 1.0f, 1.0f);
    }*/
}
