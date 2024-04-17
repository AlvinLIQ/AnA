#include "Headers/ResourceManager.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/ShaderCodes.hpp"
#include <vulkan/vulkan_core.h>

using namespace AnA;
using namespace Resource;

ResourceManager* _resourceManager = nullptr;

ResourceManager::ResourceManager(Device& mDevice) : aDevice {mDevice}, 
        SceneObjects(mDevice), 
        GlobalLight(mDevice), 
        SecondaryCommandBufferPool(mDevice, 
        VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)
{
    _resourceManager = this;
    createMainCameraBuffers();
    createShadowFramebuffers();

    createDefaultShaders();
#ifdef ANA_INCLUDE_CONTROL
    Controls::Control::InitControl(SwapChain::GetCurrent());
#endif
    TextureMap.try_emplace(DEFAULT_TEXTURE_ID, (uint32_t)0xFFFFFFFF, mDevice);
    TextureMap.try_emplace(1, (uint32_t)0xFF996666, mDevice);
    TextureMap.try_emplace(2, (uint32_t)0xFF669966, mDevice);
    TextureMap.try_emplace(3, (uint32_t)0xFF666699, mDevice);
}

ResourceManager::~ResourceManager()
{
    TextureMap.clear();
    //for (auto& shader : Shaders)
    //    delete shader;
    Shaders.clear();
    
    auto logicalDevice = aDevice.GetLogicalDevice();
    for (auto& shadowSampler : shadowSamplers)
        vkDestroySampler(logicalDevice, shadowSampler, nullptr);
    cleanupShadowResources();

    //delete SceneObjects;
    //delete GlobalLight;
#ifdef ANA_INCLUDE_CONTROL
    if (MainControl != nullptr)
        delete MainControl;
#endif

    for (auto& mainCameraBuffer : mainCameraBuffers)
        delete mainCameraBuffer;
}

ResourceManager* ResourceManager::GetCurrent()
{
    return _resourceManager;
}

std::vector<VkFramebuffer>& ResourceManager::GetShadowFramebuffers()
{
    return shadowFramebuffers;
}

void ResourceManager::UpdateCamera(float aspect)
{
    MainCameraInfo.aspect = aspect;
    MainCameraInfo.UpdateCameraPerspective(MainCamera);
    LightCameraInfo.aspect = aspect;
    //LightCameraInfo.UpdateCameraPerspective(LightCamera);
    const float scale = 10.5f;
    LightCamera.SetOrthographicProjection(-scale * LightCameraInfo.aspect, -scale, scale * LightCameraInfo.aspect, scale, 
        -10.0f, 32.0f);
}

void ResourceManager::UpdateCameraBuffer()
{
    Cameras::CameraBufferObject& cbo = *(Cameras::CameraBufferObject*)mainCameraBuffers[SwapChain::GetCurrent()->CurrentFrame]->GetMappedData();
    cbo.proj = MainCamera.GetProjectionMatrix();
    cbo.view = MainCamera.GetView();
    cbo.invView = MainCamera.GetInverseView();

    auto extent = SwapChain::GetCurrent()->GetExtent();
    cbo.resolution = {(float)extent.width, (float)extent.height};
}

void ResourceManager::Update()
{
    UpdateCameraBuffer();
    GlobalLight.UpdateBuffers(LightCamera, SwapChain::GetCurrent()->CurrentFrame);
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
    cleanupShadowResources();
    createShadowFramebuffers();
    auto deafultShadowSamplerConfig = GetDefaultDescriptorConfig()[DEFAULT_SHADOW_SAMPLER_LAYOUT];
    for (int i = 0; i < 2; i++)
    {
        Shaders[i].GetDescriptors()[DEFAULT_SHADOW_SAMPLER_LAYOUT]->UpdateDescriptorSets(deafultShadowSamplerConfig);
    }
}

std::vector<Descriptor::DescriptorConfig> ResourceManager::GetDefaultDescriptorConfig()
{
    std::vector<Descriptor::DescriptorConfig> descriptorConfigs(DEFAULT_DESCRIPTOR_SET_LAYOUT_COUNT);
    auto pConfig = &descriptorConfigs[DEFAULT_UBO_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pConfig->buffers = mainCameraBuffers.data();
    pConfig->bufferSize = sizeof(Cameras::CameraBufferObject);

    pConfig = &descriptorConfigs[DEFAULT_LIGHT_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pConfig->buffers = GlobalLight.GetBuffers();
    pConfig->bufferSize = sizeof(Lights::LightBufferObject);

    pConfig = &descriptorConfigs[DEFAULT_SAMPLER_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = 0;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pConfig->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pConfig = &descriptorConfigs[DEFAULT_SHADOW_SAMPLER_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pConfig->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pConfig->images = shadowImages.data();
    pConfig->samplers = shadowSamplers.data();

    return descriptorConfigs;
}

void ResourceManager::createMainCameraBuffers()
{
    VkDeviceSize bufferSize = sizeof(Cameras::CameraBufferObject);
    mainCameraBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto &cameraBuffer : mainCameraBuffers)
    {
        cameraBuffer = new Buffer(aDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        cameraBuffer->Map(0, bufferSize);
    }
}

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
        aDevice.CreateImage(&imageInfo, &shadowImage.image, &shadowImage.imageMemory);
        //aDevice.TransitionImageLayout(image, swapChain->GetDepthFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = shadowImage.image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = imageInfo.format;
        imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R };
        imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

        vkCreateImageView(aDevice.GetLogicalDevice(), &imageViewInfo, nullptr, &shadowImage.imageView);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &shadowImage.imageView;

        framebufferInfo.width = imageInfo.extent.width;
        framebufferInfo.height = imageInfo.extent.height;
        framebufferInfo.layers = 1;
        vkCreateFramebuffer(aDevice.GetLogicalDevice(), &framebufferInfo, nullptr, &shadowFramebuffers[i]);
        if (samplersNotCreated)
            aDevice.CreateSampler(&shadowSamplers[i], VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    }
}

void ResourceManager::cleanupShadowResources()
{
    for (auto& shadowFrameBuffer : shadowFramebuffers)
        vkDestroyFramebuffer(aDevice.GetLogicalDevice(), shadowFrameBuffer, nullptr);
    for (auto& shadowImage : shadowImages)
        shadowImage.CleanUp(aDevice.GetLogicalDevice());
}

void ResourceManager::createDefaultShaders()
{
    Shaders.reserve(3); // Reserve space for 3 default shaders
    auto renderPass = SwapChain::GetCurrent()->GetRenderPass();
    auto descriptorConfig = GetDefaultDescriptorConfig();
    Shaders.emplace_back(aDevice, Basic_vert, Basic_frag, renderPass, descriptorConfig);
    Shaders.emplace_back(aDevice, Shape_vert, Shape_frag, renderPass, descriptorConfig);
    auto offscreenRenderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();
    Shaders.emplace_back(aDevice, ShadowMapping_vert, offscreenRenderPass, descriptorConfig);
}
