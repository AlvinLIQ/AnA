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
    createDefaultDescriptors();
    createDefaultShaders();
    MainScene.Init();
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
    if (MainScene.NeedUpdate())
    {
        MainScene.Update();
    }
    if (TextContext.NeedUpdate())
    {
        TextContext.Update();
    }
    UpdateCameraBuffer();
    uint32_t frameIndex = SwapChain::GetCurrent()->CurrentFrame;
    GlobalLight.UpdateBuffers(LightCamera, frameIndex);
    ShadowMap.UpdateBuffers(MainCamera, LightCamera, frameIndex);
    if (!LockCamera)
    {
        FrustumPlanes::ExtractFrustumPlanes(ShadowMap.GetCascades().back(), *reinterpret_cast<FrustumPlanes*>(&ShadowMap.FrustumPlanes));
        memcpy(&reinterpret_cast<FrustumPlanes*>(frustumBuffers[frameIndex].GetMappedData())[1], &ShadowMap.FrustumPlanes, sizeof(FrustumPlanes));
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

void ResourceManager::GetDefaultTextDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs)
{
    descriptorSetConfigs.resize(2);
    Descriptor::DescriptorConfig config;
    config.binding = 0;
    config.descriptorCount = 0;
    config.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    config.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    if (aDevice->MeshShaderSupport())
        config.stageFlags |= VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
    descriptorSetConfigs[0].push_back(config);
    descriptorSetConfigs[1].push_back(config);
    config.binding = 1;
    descriptorSetConfigs[1].push_back(config);

    if (aDevice->MeshShaderSupport())
    {
        Descriptor::DescriptorConfig meshletConfig{};
        meshletConfig.binding = 0;
        meshletConfig.descriptorCount = 0;
        meshletConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
        Descriptor::DescriptorConfig meshletIndexConfig{};
        meshletIndexConfig.binding = 1;
        meshletIndexConfig.descriptorCount = 0;
        meshletIndexConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        meshletIndexConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;

        descriptorSetConfigs.push_back({meshletConfig, meshletIndexConfig});
    }
}

void ResourceManager::GetDefaultLightDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs)
{
    descriptorSetConfigs.resize(1);
    descriptorSetConfigs[0].resize(4);
    auto& framebuffers = SwapChain::GetCurrent()->GetOffscreenFramebuffers();
    VkSampler colorSampler = SwapChain::GetCurrent()->GetColorSampler();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& framebuffer = framebuffers[i];
        VkDescriptorImageInfo imageInfo;
        imageInfo.sampler = colorSampler;

        imageInfo.imageLayout = framebuffer.position.imageLayout;
        imageInfo.imageView = framebuffer.position.imageView;
        descriptorSetConfigs[0][0].imageInfos.push_back(imageInfo);

        imageInfo.imageLayout = framebuffer.normal.imageLayout;
        imageInfo.imageView = framebuffer.normal.imageView;
        descriptorSetConfigs[0][1].imageInfos.push_back(imageInfo);

        imageInfo.imageLayout = framebuffer.albedo.imageLayout;
        imageInfo.imageView = framebuffer.albedo.imageView;
        descriptorSetConfigs[0][2].imageInfos.push_back(imageInfo);

        imageInfo.imageLayout = framebuffer.depth.imageLayout;
        imageInfo.imageView = framebuffer.depth.imageView;
        descriptorSetConfigs[0][3].imageInfos.push_back(imageInfo);

    }
    for (uint32_t i = 0; i < 4; i++)
    {
        auto& dConfig = descriptorSetConfigs[0][i];
        dConfig.descriptorCount = 1;
        dConfig.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        dConfig.binding = i;
        dConfig.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
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
        frustumBuffer = Buffer(aDevice, 2 * sizeof(FrustumPlanes), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
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

    std::vector<std::vector<Descriptor::DescriptorConfig>> textDescriptorConfig;
    GetDefaultTextDescriptorSetConfig(textDescriptorConfig);
    Descriptor::CreateDescriptors(aDevice, textDescriptorConfig, textDescriptors);

    std::vector<std::vector<Descriptor::DescriptorConfig>> lightDescriptorConfig;
    GetDefaultLightDescriptorSetConfig(lightDescriptorConfig);
    Descriptor::CreateDescriptors(aDevice, lightDescriptorConfig, lightDescriptors);
}

std::vector<ShaderInfo> basicShaderStageInfos{{Basic_vert, 0, VK_SHADER_STAGE_VERTEX_BIT},
                                  {Basic_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT}};
std::vector<ShaderInfo> shapeShaderStageInfos{{Shape_vert, 0, VK_SHADER_STAGE_VERTEX_BIT},
                                  {Shape_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT}};
std::vector<ShaderInfo> pointShaderStageInfos{{Point_vert, 0, VK_SHADER_STAGE_VERTEX_BIT},
                                  {Point_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT}};
std::vector<ShaderInfo> csmShaderStageInfos{{CascadedShadowMapping_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT},
                                  {CascadedShadowMapping_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT}};
std::vector<ShaderInfo> terrainShaderStageInfos{{Terrain_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT},
                                  {Terrain_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT}};
std::vector<ShaderInfo> meshShaderStageInfos{{Mesh_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT},
                                  {Mesh_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT},
                                      {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false}};
std::vector<ShaderInfo> lightShaderStageInfos{{Light_vert, 0, VK_SHADER_STAGE_VERTEX_BIT},
                                {Light_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT}};                                      
std::vector<ShaderInfo> textShaderStageInfos{{Text_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT},
                                        {Text_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT},
                                            {Text_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT}};
void ResourceManager::createDefaultShaders()
{
    Shaders.reserve(8);
    Shaders.emplace_back(aDevice, basicShaderStageInfos, defaultDescriptors, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    Shaders.emplace_back(aDevice, shapeShaderStageInfos, shapeDescriptors, SHAPE_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    Shaders.emplace_back(aDevice, csmShaderStageInfos, defaultDescriptors, MESH_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    Shaders.emplace_back(aDevice, pointShaderStageInfos, defaultDescriptors, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0);

    if (aDevice->MeshShaderSupport())
    {
        Shaders.emplace_back(aDevice, terrainShaderStageInfos, defaultDescriptors, DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT, 0, sizeof(TerrainPushConstants));
        Shaders.emplace_back(aDevice, meshShaderStageInfos, defaultDescriptors, MESH_DESCRIPTOR_SET_LAYOUT_COUNT, 0, sizeof(uint32_t));
        Shaders.emplace_back(aDevice, textShaderStageInfos, textDescriptors, 3, 0, sizeof(glm::vec2));
    }
    Shaders.emplace_back(aDevice, lightShaderStageInfos, lightDescriptors, 1, 0, 0);
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
