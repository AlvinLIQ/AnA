#include "Headers/ResourceManager.hpp"
#include "../Headers/Window.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/ShaderCodes.hpp"

using namespace AnA;
using namespace Resource;

ResourceManager* _resourceManager = nullptr;

ResourceManager::ResourceManager(Device& mDevice) : aDevice {mDevice}
{
    _resourceManager = this;
    createMainCameraBuffers();
    createShadowFramebuffers();
    createTextureSampler();
    createShadowSampler();

    SceneObjects = new Objects(aDevice);
    GlobalLight = new Lights::Light(aDevice);

    createDefaultShaders();
}

ResourceManager::~ResourceManager()
{
    for (auto& shader : Shaders)
        delete shader;

    vkDestroySampler(aDevice.GetLogicalDevice(), textureSampler, nullptr);
    vkDestroySampler(aDevice.GetLogicalDevice(), shadowSampler, nullptr);
    

    delete SceneObjects;

    for (auto& mainCameraBuffer : mainCameraBuffers)
        delete mainCameraBuffer;
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
    LightCameraInfo.UpdateCameraPerspective(LightCamera);
    //LightCamera.SetOrthographicProjection(-aspect, -1.0, aspect, 1.0, LightCameraInfo.near, LightCameraInfo.far);
}

std::vector<Descriptor::DescriptorConfig> ResourceManager::GetDefault()
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
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pConfig->buffers = GlobalLight->GetBuffers();
    pConfig->bufferSize = sizeof(Lights::LightBufferObject);

    pConfig = &descriptorConfigs[DEFAULT_SSBO_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pConfig->buffers = SceneObjects->GetBuffers();
    pConfig->bufferSize = MAX_OBJECTS_SIZE;

    pConfig = &descriptorConfigs[DEFAULT_SAMPLER_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

    pConfig = &descriptorConfigs[DEFAULT_SHADOW_SAMPLER_LAYOUT];
    pConfig->binding = 0;
    pConfig->descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pConfig->images = shadowImages.data();

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
    Window* window = (Window*)glfwGetWindowUserPointer(glfwGetCurrentContext());
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& shadowImage = shadowImages[i];
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_D32_SFLOAT;
        imageInfo.extent = {static_cast<uint32_t>(window->Width), static_cast<uint32_t>(window->Height), 1};
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

        framebufferInfo.width = static_cast<uint32_t>(window->Width);
        framebufferInfo.height = static_cast<uint32_t>(window->Height);
        framebufferInfo.layers = 1;
        vkCreateFramebuffer(aDevice.GetLogicalDevice(), &framebufferInfo, nullptr, &shadowFramebuffers[i]);
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
    Shader* shader;
    auto renderPass = SwapChain::GetCurrent()->GetRenderPass();
    shader = new Shader(aDevice, Basic_vert, Basic_frag, renderPass);
    shader = new Shader(aDevice, Line_vert, Line_frag, renderPass);

}