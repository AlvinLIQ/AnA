#include "Headers/ShadowMap.hpp"
#include "../Camera/Headers/Camera.hpp"
#include "../Headers/SwapChain.hpp"
#include "../Headers/Buffer.hpp"

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

void ShadowMap::UpdateBuffers(Cameras::Camera& camera)
{
    float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

	float nearClip = 0.01f;
	float farClip = 32.0f;
	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
	}

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
		glm::mat4 invCam = glm::inverse(camera.GetProjectionMatrix() * camera.GetView());
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

		glm::vec3 lightDir = normalize(-glm::vec3(1.0, 2.0, 0.0));
		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

		// Store split distance and matrix in cascade
		cascades[i].splitDepth = (nearClip + splitDist * clipRange) * -1.0f;
		cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

		lastSplitDist = cascadeSplits[i];
	}
}

void ShadowMap::createShadowResources()
{
    shadowImages.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto& cascade : cascades)
    {
        cascade.imageViews.resize(shadowImages.size());
        cascade.framebuffers.resize(shadowImages.size());
    }
    bool samplersNotCreated = shadowSamplers.empty();
    if (samplersNotCreated)
        shadowSamplers.resize(shadowImages.size());
    auto swapChain = SwapChain::GetCurrent();
    auto extent = swapChain->GetExtent();
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
        imageInfo.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
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
        imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, SHADOW_MAP_CASCADE_COUNT };

        vkCreateImageView(aDevice->GetLogicalDevice(), &imageViewInfo, nullptr, &shadowImage.imageView);
        for (int j = 0; j < cascades.size(); j++)
        {
            auto& cascade = cascades[j];
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = swapChain->GetDepthFormat();
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = j; //layer index
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.image = shadowImages[i].image;
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
            aDevice->CreateSampler(&shadowSamplers[i], VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
		cascadeBuffer = Buffer(aDevice, cascades.size() * sizeof(Cascade), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 
			| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    }
}

void ShadowMap::cleanupShadowResources()
{
    for (auto& shadowSampler : shadowSamplers)
        vkDestroySampler(aDevice->GetLogicalDevice(), shadowSampler, nullptr);
    for (auto& cascade : cascades)
        cascade.cleanup(aDevice->GetLogicalDevice());
    for (auto& shadowImage : shadowImages)
        shadowImage.cleanup(aDevice->GetLogicalDevice());
}