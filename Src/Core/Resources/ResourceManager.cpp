#include "Headers/ResourceManager.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/ShaderCodes.hpp"
#include "Headers/Device.hpp"
#include "Resources/Headers/Animation.hpp"
#include "vulkan/vulkan_core.h"
#include <stdexcept>

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
    createSampledImageResources();
    //createShadowFramebuffers();
    createDefaultShaders();
    Meshes.Init();
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
    sampledImageDescriptor.cleanup(aDevice->GetLogicalDevice());
    samplerDescriptor.cleanup(aDevice->GetLogicalDevice());
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
    auto& mbo = *reinterpret_cast<MiscBufferObject*>(miscBuffers[bufferIndex].GetMappedData());
    mbo.objectCount = uint32_t(MainScene.GetMeshCount());
    mbo.meshletCount = MainScene.GetMeshletCount();
    mbo.meshletIDCount = MainScene.GetMeshletIDCount();
    mbo.viewProj = MainCamera.GetViewProj();
    if (!LockCamera)
    {
        FrustumPlanes::ExtractFrustumPlanes(MainCamera.GetViewProj(), MainCameraFrustumPlanes);
        memcpy(&mbo.planes,
            &MainCameraFrustumPlanes,
            sizeof(FrustumPlanes));
        mbo.cameraPosition = MainCamera.GetInverseView()[3];
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

void ResourceManager::BindDescriptors(VkCommandBuffer commandBuffer)
{
    if (aDevice->DescriptorHeapSupport())
    {
        auto& prop = aDevice->GetDescriptorHeapProperties();
        VkBindHeapInfoEXT bindInfo{};
        bindInfo.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
        bindInfo.reservedRangeOffset = 0;
        bindInfo.reservedRangeSize = prop.minSamplerHeapReservedRange;
        bindInfo.heapRange.address = samplerDescriptor.buffer.GetAddress();
        bindInfo.heapRange.size = samplerDescriptor.buffer.GetSize();
        vkCmdBindSamplerHeapEXT(commandBuffer, &bindInfo);

        bindInfo.reservedRangeOffset = 0;
        bindInfo.reservedRangeSize = prop.minResourceHeapReservedRange;
        bindInfo.heapRange.address = sampledImageDescriptor.buffer.GetAddress();
        bindInfo.heapRange.size = sampledImageDescriptor.buffer.GetSize();
        vkCmdBindResourceHeapEXT(commandBuffer, &bindInfo);
    }
    else if (aDevice->DescriptorBufferSupport())
    {
        VkDescriptorBufferBindingInfoEXT bindInfos[2]{};
        bindInfos[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
        bindInfos[0].address = samplerDescriptor.buffer.GetAddress();
        bindInfos[0].usage = samplerDescriptor.buffer.GetUsage();

        bindInfos[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
        bindInfos[1].address = sampledImageDescriptor.buffer.GetAddress();
        bindInfos[1].usage = sampledImageDescriptor.buffer.GetUsage();
        vkCmdBindDescriptorBuffersEXT(commandBuffer, numsof(bindInfos), bindInfos);
    }
}

void ResourceManager::BindDescriptors(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    VkDescriptorSet sets[] = {samplerDescriptor.set, sampledImageDescriptor.set};
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
        0, numsof(sets), sets, 0, VK_NULL_HANDLE);
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

    appendSampledImage(result.first->first, result.first->second);

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
    {
        *index = aDevice->DescriptorHeapSupport() ? uint32_t(textureHeapInfos.size()) :
            uint32_t(textureInfos.size());
    }
    TexturePathMap.emplace(path, texId);

    appendSampledImage(result.first->first, result.first->second);

    texId++;
    return result.first->first;
}

void ResourceManager::createMiscBuffers()
{
    uint32_t imageCount = SwapChain::GetCurrent()->GetImageCount();
    miscBuffers.resize(imageCount);
    for (auto& miscBuffer : miscBuffers)
    {
        miscBuffer = Buffer(aDevice,
            sizeof(MiscBufferObject),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        miscBuffer.Map();
    }
    //MainCamera.SetRotateSpeed(float(imageCount) * 1.5f);

}

void ResourceManager::createSampledImageResources()
{
    if (aDevice->DescriptorHeapSupport())
    {
        const auto& prop = aDevice->GetDescriptorHeapProperties();
        samplerDescriptor.buffer = Buffer(aDevice,
            prop.samplerDescriptorSize +
            prop.minSamplerHeapReservedRange,
            VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            prop.samplerDescriptorAlignment);
        samplerDescriptor.buffer.Map();

        sampledImageDescriptor.buffer = Buffer(aDevice,
            MaxBatchSize * prop.imageDescriptorSize +
            prop.minResourceHeapReservedRange,
            VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            prop.resourceHeapAlignment);
        sampledImageDescriptor.buffer.Map();
    }
    else
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.binding = 0;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;
        if(aDevice->DescriptorBufferSupport())
            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

        vkCreateDescriptorSetLayout(aDevice->GetLogicalDevice(), &layoutInfo,
            VK_NULL_HANDLE, &samplerDescriptor.setLayout);

        binding.descriptorCount = MaxBatchSize;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.binding = 0;

        vkCreateDescriptorSetLayout(aDevice->GetLogicalDevice(), &layoutInfo,
            VK_NULL_HANDLE, &sampledImageDescriptor.setLayout);

        if(aDevice->DescriptorBufferSupport())
        {
            const auto& prop = aDevice->GetDescriptorBufferProperties();
            samplerDescriptor.buffer = Buffer(aDevice,
                prop.samplerDescriptorSize,
                VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
            samplerDescriptor.buffer.Map();

            sampledImageDescriptor.buffer = Buffer(aDevice,
                MaxBatchSize * prop.sampledImageDescriptorSize,
                VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
            sampledImageDescriptor.buffer.Map();
        }
        else
        {
            //create descriptor set here for traditional device
            VkDescriptorPoolSize poolSizes[2];
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
            poolSizes[0].descriptorCount = 1;
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            poolSizes[1].descriptorCount = MaxBatchSize;
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.pPoolSizes = poolSizes;
            poolInfo.maxSets = 2;
            poolInfo.poolSizeCount = numsof(poolSizes);
            if (vkCreateDescriptorPool(aDevice->GetLogicalDevice(), &poolInfo, VK_NULL_HANDLE, &samplerDescriptor.pool) != VK_SUCCESS)
                throw std::runtime_error("failed to create descriptor pool");

            VkDescriptorSetLayout setLayouts[] = {samplerDescriptor.setLayout, sampledImageDescriptor.setLayout};
            VkDescriptorSet sets[2];
            VkDescriptorSetAllocateInfo allocInfo;
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = VK_NULL_HANDLE;
            allocInfo.pSetLayouts = setLayouts;
            allocInfo.descriptorPool = samplerDescriptor.pool;
            allocInfo.descriptorSetCount = 2;
            if (vkAllocateDescriptorSets(aDevice->GetLogicalDevice(), &allocInfo, sets) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate descriptor sets");
            samplerDescriptor.set = sets[0];
            sampledImageDescriptor.set = sets[1];

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            write.dstSet = sets[0];

            VkDescriptorImageInfo samplerInfo{};
            samplerInfo.sampler = SwapChain::GetCurrent()->GetColorSampler();
            write.pImageInfo = &samplerInfo;
            vkUpdateDescriptorSets(aDevice->GetLogicalDevice(), 1, &write, 0, VK_NULL_HANDLE);
        }
    }
}

void ResourceManager::appendSampledImage(uint32_t id, Texture& texture)
{
    if (aDevice->DescriptorHeapSupport())
    {
        TextureIdMap.emplace(id, uint32_t(textureHeapInfos.size()));
        appendSampledImage(texture.GetImageHeapInfo());
    }
    else
    {
        TextureIdMap.emplace(id, uint32_t(textureInfos.size()));
        appendSampledImage(texture.GetImageInfo());
    }
}

void ResourceManager::appendSampledImage(VkImageDescriptorInfoEXT& imageInfo)
{
    const auto& prop = aDevice->GetDescriptorHeapProperties();

    VkResourceDescriptorInfoEXT resourceDescriptor{};
    resourceDescriptor.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT;
    resourceDescriptor.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    resourceDescriptor.data.pImage = &imageInfo;

    VkHostAddressRangeEXT descriptorRange{};
    descriptorRange.address = reinterpret_cast<uint8_t*>(sampledImageDescriptor.buffer.GetMappedData()) +
        textureHeapInfos.size() * prop.imageDescriptorSize;// + prop.minResourceHeapReservedRange;
    descriptorRange.size = prop.imageDescriptorSize;

    vkWriteResourceDescriptorsEXT(aDevice->GetLogicalDevice(),
        1,
        &resourceDescriptor,
        &descriptorRange);

    textureHeapInfos.push_back(imageInfo);
}

void ResourceManager::appendSampledImage(VkDescriptorImageInfo& imageInfo)
{
    if (aDevice->DescriptorBufferSupport())
    {
        const auto& prop = aDevice->GetDescriptorBufferProperties();
        uint8_t* dst = reinterpret_cast<uint8_t*>(sampledImageDescriptor.buffer.GetMappedData()) +
            (textureInfos.size()) * prop.sampledImageDescriptorSize;

        VkDescriptorGetInfoEXT getInfo{};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        getInfo.data.pSampledImage = &imageInfo;
        vkGetDescriptorEXT(aDevice->GetLogicalDevice(), &getInfo, prop.sampledImageDescriptorSize, dst);
    }
    else
    {
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.pImageInfo = &imageInfo;
        write.dstBinding = 0;
        write.dstSet = sampledImageDescriptor.set;
        write.dstArrayElement = uint32_t(textureInfos.size());

        vkUpdateDescriptorSets(aDevice->GetLogicalDevice(), 1, &write, 0, VK_NULL_HANDLE);
    }

    textureInfos.push_back(imageInfo);
}

/*
std::vector<ShaderInfo> basicShaderStageInfos{{Basic_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Basic_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> pointShaderStageInfos{{Point_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Point_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> csmShaderStageInfos{{CascadedShadowMapping_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                  {CascadedShadowMapping_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}}};*/
std::vector<ShaderInfo> shapeShaderStageInfos{{Shape_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Shape_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::initializer_list<int> shaderConstants{DEFAULT_CULL};
std::vector<ShaderInfo> meshShaderStageInfos{{Mesh_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, shaderConstants},
                                  {Mesh_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, shaderConstants},
                                      {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> meshVertexShaderStageInfos{{Mesh_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
/*
std::vector<ShaderInfo> lightShaderStageInfos{{Light_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                {Light_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};*/
std::vector<ShaderInfo> textShaderStageInfos{{Text_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                        {Text_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}},
                                            {Text_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> textVertexShaderStageInfos{{Text_vert, 0, VK_SHADER_STAGE_VERTEX_BIT, false, {}},
                                  {Text_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
/*
std::vector<ShaderInfo> terrainShaderStageInfos{{Terrain_task, 0, VK_SHADER_STAGE_TASK_BIT_EXT, false, {}},
                                        {Terrain_mesh, 0, VK_SHADER_STAGE_MESH_BIT_EXT, false, {}},
                                            {Mesh_frag, 0, VK_SHADER_STAGE_FRAGMENT_BIT, false, {}}};
std::vector<ShaderInfo> collisionShaderStageInfos{{CollisionDetect_comp, 0, VK_SHADER_STAGE_COMPUTE_BIT, false, {}}};*/

void ResourceManager::createDefaultShaders()
{
    Shaders.reserve(10);
    std::vector<VkDescriptorSetLayout> emptyLayout{};
    std::vector<VkDescriptorSetLayout> textureLayouts{samplerDescriptor.setLayout, sampledImageDescriptor.setLayout};

    auto& prop = aDevice->GetDescriptorHeapProperties();
    VkDescriptorSetAndBindingMappingEXT mappings[2]{};
    mappings[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
    mappings[0].firstBinding = 0;
    mappings[0].bindingCount = 1;
    mappings[0].descriptorSet = 0;
    mappings[0].resourceMask = VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT;
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

    mappings[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
    mappings[1].firstBinding = 0;
    mappings[1].bindingCount = 1;
    mappings[1].descriptorSet = 1;
    mappings[1].resourceMask = VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT;
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapArrayStride = prop.imageDescriptorSize;

    VkShaderDescriptorSetAndBindingMappingInfoEXT descriptorSetMappingInfo{};
    descriptorSetMappingInfo.sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT;
    descriptorSetMappingInfo.mappingCount = numsof(mappings);
    descriptorSetMappingInfo.pMappings = mappings;

    if (aDevice->MeshShaderSupport())
    {
        Shaders.emplace_back(aDevice, meshShaderStageInfos, textureLayouts, sizeof(MeshPushConstant), &descriptorSetMappingInfo);
        Shaders.emplace_back(aDevice, textShaderStageInfos, emptyLayout, sizeof(TextPushConstant));
    }
    else
    {
        Shaders.emplace_back(aDevice, meshVertexShaderStageInfos, textureLayouts, sizeof(MeshVertexPushConstant), &descriptorSetMappingInfo);
        Shaders.emplace_back(aDevice, textVertexShaderStageInfos, emptyLayout, sizeof(TextPushConstant));
    }
    Shaders.emplace_back(aDevice, shapeShaderStageInfos, textureLayouts, sizeof(ShapePushConstant), &descriptorSetMappingInfo);
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
    if (aDevice->DescriptorHeapSupport())
    {
        auto& prop = aDevice->GetDescriptorHeapProperties();
        auto samplerInfo = aDevice->SamplerInfo();
        VkHostAddressRangeEXT descriptorRange;
        descriptorRange.address = reinterpret_cast<uint8_t*>(samplerDescriptor.buffer.GetMappedData());// +
            //prop.minSamplerHeapReservedRange;
        descriptorRange.size = prop.samplerDescriptorSize;
        vkWriteSamplerDescriptorsEXT(aDevice->GetLogicalDevice(), 1, &samplerInfo, &descriptorRange);
    }
    else if (aDevice->DescriptorBufferSupport())
    {
        auto& prop = aDevice->GetDescriptorBufferProperties();
        uint8_t* dst = reinterpret_cast<uint8_t*>(samplerDescriptor.buffer.GetMappedData());
        VkDescriptorGetInfoEXT getInfo{};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLER;
        getInfo.data.pSampler = &SwapChain::GetCurrent()->GetColorSampler();
        vkGetDescriptorEXT(aDevice->GetLogicalDevice(), &getInfo, prop.samplerDescriptorSize, dst);
    }
    for (auto& color : defaultTextureColors)
    {
        AppendTexture(color);
    }

    aDevice->BuildFontVertices(Characters);
}
