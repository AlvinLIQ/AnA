#include "Headers/ResourceManager.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/ShaderCodes.hpp"
#include <vulkan/vulkan_core.h>

using namespace AnA;
using namespace Resource;

ResourceManager* _resourceManager = nullptr;

ResourceManager::ResourceManager(Device* mDevice) : 
        SceneObjects(mDevice),
        GlobalLight(mDevice),
        ShadowMap(mDevice),
        SecondaryCommandBufferPool(mDevice, 
        VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)
{
    aDevice = mDevice;
    _resourceManager = this;
    createMainCameraBuffers();
    //createShadowFramebuffers();
    createDefaultShaders();
    Shapes = AnA::Shapes(mDevice);
    //Shapes = new AnA::Shapes(mDevice);
#ifdef ANA_INCLUDE_CONTROL
    Controls::Control::InitControl(SwapChain::GetCurrent());
#endif
    TextureMap.try_emplace(DEFAULT_TEXTURE_ID, (uint32_t)0xFFFFFFFF, mDevice);
    TextureMap.try_emplace(1, (uint32_t)0xFFCC9999, mDevice);
    TextureMap.try_emplace(2, (uint32_t)0xFF99CC99, mDevice);
    TextureMap.try_emplace(3, (uint32_t)0xFF9999CC, mDevice);
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

    //delete SceneObjects;
    //delete GlobalLight;
#ifdef ANA_INCLUDE_CONTROL
    delete MainControl;
#endif
}

ResourceManager* ResourceManager::GetCurrent()
{
    return _resourceManager;
}

std::vector<Cascade>& ResourceManager::GetCascades()
{
    return ShadowMap.GetCascades();
    //return shadowFramebuffers;
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
    const float scale = 10.5f;
    //LightCamera.SetOrthographicProjection(-scale * LightCameraInfo.aspect, -scale, scale * LightCameraInfo.aspect, scale, 
    //    -10.0f, 32.0f);
    LightCamera.SetOrthographicProjection(-scale * LightCameraInfo.aspect, -scale, scale * LightCameraInfo.aspect, scale, 
        -10.0f, 32.0f);
}

void ResourceManager::UpdateCameraBuffer()
{
    Cameras::CameraBufferObject& cbo = *(Cameras::CameraBufferObject*)mainCameraBuffers[SwapChain::GetCurrent()->CurrentFrame].GetMappedData();
    cbo.proj = MainCamera.GetProjectionMatrix();
    cbo.view = MainCamera.GetView();
    //cbo.invView = MainCamera.GetInverseView();

    auto extent = SwapChain::GetCurrent()->GetExtent();
    cbo.resolution = {(float)extent.width, (float)extent.height};
}

void ResourceManager::Update()
{
    UpdateCameraBuffer();
    uint32_t frameIndex = SwapChain::GetCurrent()->CurrentFrame;
    GlobalLight.UpdateBuffers(LightCamera, frameIndex);
    ShadowMap.UpdateBuffers(MainCamera, LightCamera, frameIndex);
    if (SceneObjects.NeedUpdate())
    {
        SceneObjects.CommitBufferUpdate();
    }
    for (auto& recordCallBackInfo : RecordCallBacks)
    {
        if (recordCallBackInfo.needRecord())
        {
            recordCallBackInfo.recordCallBack(recordCallBackInfo.offset, recordCallBackInfo.extent);
        }
    }
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
    std::vector<std::vector<Descriptor::DescriptorConfig>> configs;
    GetDefaultDescriptorSetConfig(configs);
    auto deafultShadowSamplerConfig = configs[DEFAULT_SHADOW_SAMPLER_LAYOUT].begin();
    for (int i = 0; i < 1; i++)
    {
        Shaders[i].GetDescriptors()[DEFAULT_SHADOW_SAMPLER_LAYOUT]->UpdateDescriptorSets(*deafultShadowSamplerConfig);
    }
}

void ResourceManager::GetDefaultDescriptorSetConfig(std::vector<std::vector<Descriptor::DescriptorConfig>>& descriptorSetConfigs)
{
    descriptorSetConfigs.resize(DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT);
    for (auto& configs : descriptorSetConfigs)
        configs.resize(1);
    auto pConfig = descriptorSetConfigs[DEFAULT_VERTEX_LAYOUT].begin();
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT;
/*
    pConfig = &descriptorConfigs[DEFAULT_MESHLET_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT;
*/
    pConfig = descriptorSetConfigs[DEFAULT_UBO_LAYOUT].begin();
    pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT;
    GetBufferInfos(mainCameraBuffers, pConfig->bufferInfos);

    pConfig = descriptorSetConfigs[DEFAULT_LIGHT_LAYOUT].begin();
    pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT;
    GetBufferInfos(GlobalLight.GetBuffers(), pConfig->bufferInfos);

    pConfig = descriptorSetConfigs[DEFAULT_SAMPLER_LAYOUT].begin();
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pConfig->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pConfig = descriptorSetConfigs[DEFAULT_SHADOW_SAMPLER_LAYOUT].begin();
    pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pConfig->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    GetImageInfos(ShadowMap.GetImages(), ShadowMap.GetSamplers(), pConfig->imageInfos);

    ShadowMap.GetUBODescriptorConfig(&descriptorSetConfigs[DEFAULT_CASCADED_UBO_LAYOUT][0]);
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

void ResourceManager::createMainCameraBuffers()
{
    VkDeviceSize bufferSize = sizeof(Cameras::CameraBufferObject);
    mainCameraBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto &cameraBuffer : mainCameraBuffers)
    {
        cameraBuffer = Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        cameraBuffer.Map(0, bufferSize);
    }
}
/*
void ResourceManager::createShadowFramebuffers()
{
    shadowImages.resize(MAX_FRAMES_IN_FLIGHT);
    shadowFramebuffers.resize(MAX_FRAMES_IN_FLIGHT);
    bool samplersNotCreated = shadowSamplers.empty();
    if (samplersNotCreated)
        shadowSamplers.resize(MAX_FRAMES_IN_FLIGHT);
    auto extent = SwapChain::GetCurrent()->GetExtent();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& shadowImage = shadowImages[i];
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_D32_SFLOAT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.extent = {extent.width, extent.height, 1};
        shadowImage.extent = imageInfo.extent;
        shadowImage.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        aDevice->CreateImage(&imageInfo, &shadowImage.image, &shadowImage.imageMemory);
        //aDevice->TransitionImageLayout(image, swapChain->GetDepthFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = shadowImage.image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = imageInfo.format;
        imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R };
        imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

        vkCreateImageView(aDevice->GetLogicalDevice(), &imageViewInfo, nullptr, &shadowImage.imageView);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &shadowImage.imageView;

        framebufferInfo.width = imageInfo.extent.width;
        framebufferInfo.height = imageInfo.extent.height;
        framebufferInfo.layers = 1;
        vkCreateFramebuffer(aDevice->GetLogicalDevice(), &framebufferInfo, nullptr, &shadowFramebuffers[i]);
        if (samplersNotCreated)
            aDevice->CreateSampler(&shadowSamplers[i], VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    }
}

void ResourceManager::cleanupShadowResources()
{
    for (auto& shadowFrameBuffer : shadowFramebuffers)
        vkDestroyFramebuffer(aDevice->GetLogicalDevice(), shadowFrameBuffer, nullptr);
    for (auto& shadowImage : shadowImages)
        shadowImage.cleanup(aDevice->GetLogicalDevice());
}
*/
void ResourceManager::createDefaultShaders()
{
    Shaders.reserve(5); // Reserve space for 3 default shaders
    auto renderPass = SwapChain::GetCurrent()->GetRenderPass();
    std::vector<std::vector<Descriptor::DescriptorConfig>> descriptorConfig;
    GetDefaultDescriptorSetConfig(descriptorConfig);
    Shaders.emplace_back(aDevice, Basic_vert, Basic_frag, renderPass, descriptorConfig);

    std::vector<std::vector<Descriptor::DescriptorConfig>> shapesDescriptorConfig;
    GetDefaultShapesDescriptorSetConfig(shapesDescriptorConfig);
    Shaders.emplace_back(aDevice, Shape_vert, Shape_frag, renderPass, shapesDescriptorConfig);

    auto offscreenRenderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();

    Shaders.emplace_back(aDevice, CascadedShadowMapping_vert, offscreenRenderPass
        , CascadedShadowMapping_frag, descriptorConfig, sizeof(uint32_t));

    
    Descriptor::DescriptorConfig meshletConfig{};
    meshletConfig.binding = 0;
    meshletConfig.descriptorCount = 0;
    meshletConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    meshletConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
    Descriptor::DescriptorConfig meshletCullingConfig{};
    meshletCullingConfig.binding = 1;
    meshletCullingConfig.descriptorCount = 0;
    meshletCullingConfig.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    meshletCullingConfig.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT;

    descriptorConfig.push_back({meshletConfig, meshletCullingConfig});

    Shaders.emplace_back(aDevice, Mesh_task, Mesh_mesh, Mesh_frag, renderPass
        , descriptorConfig);
    //std::vector<Descriptor::DescriptorConfig> emptyConfig{};
}
