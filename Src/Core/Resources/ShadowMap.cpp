#include "Headers/ShadowMap.hpp"
#include "../Camera/Headers/Camera.hpp"
#include "../Headers/SwapChain.hpp"

using namespace AnA;
using namespace Resource;

ShadowMap::ShadowMap(Device* mDevice) : aDevice{mDevice}
{
    createShadowResources();
}

ShadowMap::~ShadowMap()
{
    cleanupShadowResources();
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view, glm::vec3& center)
{
    const auto inv = glm::inverse(proj * view);
    
    std::vector<glm::vec4> frustumCorners;
    frustumCorners.reserve(8);
    center = glm::vec3(0, 0, 0);
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = 
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.emplace_back(pt / pt.w);
                center += glm::vec3(frustumCorners.back());
            }
        }
    }
    center /= frustumCorners.size();
    
    return frustumCorners;
}

void ShadowMap::UpdateBuffers(Cameras::Camera& camera, Cameras::Camera& light, uint32_t bufferIndex)
{
	//glm::vec3 lightDir = glm::normalize(-glm::vec3(1.0f, 1.0f, 1.0f));
    //glm::vec3 lightPos = lightDir * 2.0f;
    //float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
    
	float nearPlane = 0.05f;
	float farPlane = 32.0f;
    float clipRange = farPlane - nearPlane;
    glm::mat4 invCam = glm::inverse(camera.GetProjectionMatrix() * camera.GetView());
	const glm::vec3 frustumCorners[8] =
    {
        {-1.0f, -1.0f, 0.0f},
        {1.0f, -1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {-1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
    };
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
    {
        glm::vec4 frustumCenter = {};
        for (int j = 0; j < 8; j++)
        {
            frustumCenter += invCam * glm::vec4(frustumCorners[j], 1.0f);
        }
        frustumCenter /= 8.0f;
        //light.SetViewDirection(lightDir * (float)i,  lightDir, glm::vec3(0, -1, 0));

        float splitDist = (float)i / (float)SHADOW_MAP_CASCADE_COUNT;
		auto cbo = (CascadeBufferObject*)cascadeBuffers[bufferIndex].GetMappedData();
        cbo[i].viewProjMatrix = light.GetProjectionMatrix() * light.GetView();
        cbo[i].splitDepth = (nearPlane + splitDist * clipRange) * -1.0f;
	}   
}

void ShadowMap::GetUBODescriptorConfig(Descriptor::DescriptorConfig* pConfig)
{
	*pConfig = {};
	pConfig->binding = 0;
    pConfig->descriptorCount = cascadeBuffers.size();
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pConfig->buffers = cascadeBuffers.data();
    pConfig->bufferSize = cascadeBuffers.begin()->GetSize();
}

void ShadowMap::createShadowResources()
{
    images.resize(MAX_FRAMES_IN_FLIGHT);
	cascades.resize(SHADOW_MAP_CASCADE_COUNT);
    for (auto& cascade : cascades)
    {
        cascade.imageViews.resize(images.size());
        cascade.framebuffers.resize(images.size());
    }
	cascadeBuffers.reserve(images.size());
    descriptorImageInfos.resize(images.size());
    bool samplersNotCreated = samplers.empty();
    if (samplersNotCreated)
        samplers.resize(images.size());
    auto swapChain = SwapChain::GetCurrent();
    for (size_t i = 0; i < images.size(); i++)
    {
        auto& shadowImage = images[i];
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = swapChain->GetDepthFormat();
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.extent = {SHADOW_MAP_DIM, SHADOW_MAP_DIM, 1};
        shadowImage.extent = imageInfo.extent;
        shadowImage.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        aDevice->CreateImage(&imageInfo, &shadowImage.image, &shadowImage.imageMemory);

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = shadowImage.image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        imageViewInfo.format = imageInfo.format;
        //imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R };
        imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, SHADOW_MAP_CASCADE_COUNT };

        vkCreateImageView(aDevice->GetLogicalDevice(), &imageViewInfo, nullptr, &shadowImage.imageView);
        for (uint32_t j = 0; j < cascades.size(); j++)
        {
            auto& cascade = cascades[j];
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = swapChain->GetDepthFormat();
			viewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, j, 1 };
			viewInfo.image = shadowImage.image;
			vkCreateImageView(aDevice->GetLogicalDevice(), &viewInfo, nullptr, &cascade.imageViews[i]);

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &cascade.imageViews[i];

            framebufferInfo.width = imageInfo.extent.width;
            framebufferInfo.height = imageInfo.extent.height;
            framebufferInfo.layers = 1;
            vkCreateFramebuffer(aDevice->GetLogicalDevice(), &framebufferInfo, nullptr, &cascade.framebuffers[i]);
        }
        if (samplersNotCreated)
            aDevice->CreateSampler(&samplers[i], VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
		cascadeBuffers.emplace_back(aDevice, cascades.size() * sizeof(CascadeBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 
				| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		cascadeBuffers[i].Map(0, cascades.size() * sizeof(CascadeBufferObject));

        descriptorImageInfos[i].imageLayout = shadowImage.imageLayout;
        descriptorImageInfos[i].imageView = shadowImage.imageView;
        descriptorImageInfos[i].sampler = samplers[i];
    }
}

void ShadowMap::cleanupShadowResources()
{
    for (auto& sampler : samplers)
        vkDestroySampler(aDevice->GetLogicalDevice(), sampler, nullptr);
    for (auto& cascade : cascades)
        cascade.cleanup(aDevice->GetLogicalDevice());
    for (auto& shadowImage : images)
        shadowImage.cleanup(aDevice->GetLogicalDevice());
}