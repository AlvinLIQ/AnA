#include "Headers/ShadowMap.hpp"
#include "Headers/ResourceManager.hpp"
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

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& inv, glm::vec3& center)
{
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
    center /= float(frustumCorners.size());

    return frustumCorners;
}

void ShadowMap::UpdateBuffers(Cameras::Camera& camera, Cameras::Camera& light, uint32_t bufferIndex)
{
	glm::vec3 lightDir = glm::normalize(-Resource::ResourceManager::GetCurrent()->GlobalLight.Direction);
    //glm::vec3 lightPos = lightDir * 2.0f;
    //float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

	float nearPlane = 2.5f;
	float farPlane = 28.0f;
    float clipRange = farPlane - nearPlane;
    float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

	float minZ = nearPlane;
	float maxZ = nearPlane + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;
	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearPlane) / clipRange;
	}

    glm::mat4 invCam = glm::inverse(camera.GetProjectionMatrix() * camera.GetView());
    //float aspect = Resource::ResourceManager::GetCurrent()->MainCameraInfo.aspect;

	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		float splitDist = cascadeSplits[i];
		glm::vec3 frustumCorners[8] = {
			glm::vec3(-1.0f,  1.0f, 0.0f),
			glm::vec3( 1.0f,  1.0f, 0.0f),
			glm::vec3( 1.0f, -1.0f, 0.0f),
			glm::vec3(-1.0f, -1.0f, 0.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3( 1.0f,  1.0f,  1.0f),
			glm::vec3( 1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
		};
		// Project frustum corners into world space
		for (uint32_t j = 0; j < 8; j++) {
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
			frustumCorners[j] = invCorner / invCorner.w;
		}
		for (uint32_t j = 0; j < 4; j++) {
			glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
			frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
			frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
		}
		// Get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t j = 0; j < 8; j++) {
			frustumCenter += frustumCorners[j];
		}
		frustumCenter /= 8.0f;
		float radius = 0.0f;
		for (uint32_t j = 0; j < 8; j++) {
			float distance = glm::length(frustumCorners[j] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;
		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;
		//glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
		// Store split distance and matrix in cascade
        light.SetViewTarget(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0, -1, 0));
        light.SetOrthographicProjection(minExtents.x, minExtents.y, maxExtents.x, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
		auto cbo = (CascadeBufferObject*)cascadeBuffers[bufferIndex].GetMappedData();
        cbo[i].viewProjMatrix = light.GetProjectionMatrix() * light.GetView();
        cbo[i].splitDepth = (nearPlane + splitDist * clipRange);

	    lastSplitDist = cascadeSplits[i];
	}
}

void ShadowMap::GetUBODescriptorConfig(Descriptor::DescriptorConfig* pConfig)
{
	*pConfig = {};
	pConfig->binding = 0;
    pConfig->descriptorCount = 1;
    pConfig->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pConfig->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT;
    Resource::ResourceManager::GetBufferInfos(cascadeBuffers, pConfig->bufferInfos);
}

void ShadowMap::createShadowResources()
{
    images.resize(MAX_FRAMES_IN_FLIGHT);
    framebuffers.resize(images.size());
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
        aDevice->CreateImage(&imageInfo, &shadowImage.image, shadowImage.allocation);

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = shadowImage.image;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        imageViewInfo.format = imageInfo.format;
        //imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R };
        imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, SHADOW_MAP_CASCADE_COUNT };

        vkCreateImageView(aDevice->GetLogicalDevice(), &imageViewInfo, nullptr, &shadowImage.imageView);
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = SwapChain::GetCurrent()->GetOffscreenRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &shadowImage.imageView;

        framebufferInfo.width = imageInfo.extent.width;
        framebufferInfo.height = imageInfo.extent.height;
        framebufferInfo.layers = SHADOW_MAP_CASCADE_COUNT;
        vkCreateFramebuffer(aDevice->GetLogicalDevice(), &framebufferInfo, nullptr, &framebuffers[i]);

        if (samplersNotCreated)
            aDevice->CreateSampler(&samplers[i], VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
		cascadeBuffers.emplace_back(aDevice, SHADOW_MAP_CASCADE_COUNT * sizeof(CascadeBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
		cascadeBuffers[i].Map();

        descriptorImageInfos[i].imageLayout = shadowImage.imageLayout;
        descriptorImageInfos[i].imageView = shadowImage.imageView;
        descriptorImageInfos[i].sampler = samplers[i];
    }
}

void ShadowMap::cleanupShadowResources()
{
    for (auto& sampler : samplers)
        vkDestroySampler(aDevice->GetLogicalDevice(), sampler, nullptr);
    for (auto& framebuffer : framebuffers)
        vkDestroyFramebuffer(aDevice->GetLogicalDevice(), framebuffer, nullptr);
    for (auto& shadowImage : images)
        shadowImage.cleanup(aDevice);
}
