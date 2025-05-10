#include "Headers/Device.hpp"
#include "Headers/Instance.hpp"

#include <set>
#include <stdexcept>

#ifdef INCLUDE_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "Headers/Buffer.hpp"
#include "../3rdParty/stb/stb_image.h"
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "../3rdParty/stb/stb_truetype.h"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

using namespace AnA;

Device::Device(VkInstance &mInstance, VkSurfaceKHR &mSurface) : instance {mInstance}, surface {mSurface}
{
    pickPhysicalDevice();
    createLogicalDevice();
    createVmaAllocator();
    CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &commandPool);
}

Device::~Device()
{
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(logicalDevice, nullptr);
}

void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage, VkBuffer& buffer, VmaAllocation& allocation)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memUsage;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    if (usage == VK_BUFFER_USAGE_TRANSFER_DST_BIT || usage == VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;

    vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
}

void Device::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
{
    if (!allocation)
        return;
    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Device::MapBuffer(void** data, VmaAllocation allocation)
{
    vmaMapMemory(allocator, allocation, data);
}

void Device::UnmapBuffer(VmaAllocation allocation)
{
    vmaUnmapMemory(allocator, allocation);
}

void Device::FlushAllocation(VmaAllocation allocation)
{
    vmaFlushAllocation(allocator, allocation, 0, VK_WHOLE_SIZE);
}

void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* copyRegions)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, copyRegions);

    endSingleTimeCommands(commandBuffer);
}

void Device::CopyBufferToImage(VkBuffer srcBuffer, VkImage& dstImage, VkExtent3D extent)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = extent;

    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);
    endSingleTimeCommands(commandBuffer);
}

void Device::CreateImage(VkImageCreateInfo* pCreateInfo, VkImage* pImage, VmaAllocation& allocation)
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(allocator, pCreateInfo, &allocInfo, pImage, &allocation, nullptr);
}

void Device::DestroyImage(VkImage image, VmaAllocation allocation)
{
    vmaDestroyImage(allocator, image, allocation);
}

VkImageView Device::CreateImageView(VkImage& image, VkFormat format, VkImageViewType viewType, VkImageSubresourceRange subresourceRange)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;

    createInfo.viewType = viewType;
    createInfo.format = format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange = subresourceRange;

    VkImageView imageView;
    if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Device::CreateColorImage(const uint32_t color, VkImage* pTexImage, VmaAllocation& allocation)
{
    Buffer aBuffer(this, sizeof(color), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    aBuffer.Map();
    memcpy(aBuffer.GetMappedData(), &color, sizeof(color));
    aBuffer.Unmap();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = 1;
    imageInfo.extent.height = 1;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    CreateImage(&imageInfo, pTexImage, allocation);

    TransitionImageLayout(*pTexImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(aBuffer.GetBuffer(), *pTexImage, imageInfo.extent);
    TransitionImageLayout(*pTexImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

#ifdef INCLUDE_STB_IMAGE
void Device::CreateTextureImage(const char* imagePath, VkImage* pTexImage, VmaAllocation& allocation)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(imagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

    if (!pixels)
        throw std::runtime_error("Failed to load texture image!");

    Buffer aBuffer(this, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    aBuffer.Map();
    memcpy(aBuffer.GetMappedData(), pixels, static_cast<size_t>(imageSize));
    aBuffer.Unmap();
    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(texWidth);
    imageInfo.extent.height = static_cast<uint32_t>(texHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    CreateImage(&imageInfo, pTexImage, allocation);

    TransitionImageLayout(*pTexImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(aBuffer.GetBuffer(), *pTexImage, imageInfo.extent);
    TransitionImageLayout(*pTexImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Device::CreateTextImage(const char* text, int& width, int& height, float lineHeight, VkImage* pTextImage, VmaAllocation& allocation, float scaleX, float scaleY)
{
    auto fontData = ReadFile("Fonts/SourceCodePro-Black.otf");
    stbtt_fontinfo info{};
    if (!stbtt_InitFont(&info, const_cast<const unsigned char*>(fontData.data()), 0))
        throw std::runtime_error("failed to init font");

    int imageSize = static_cast<int>(static_cast<float>(width * height) * scaleX * scaleY);
    if (lineHeight <= 0.0f)
    {
        lineHeight = ANA_TEXT_DEFAULT_LINE_HEIGHT;
    }
    lineHeight *= scaleY;

    float scale = stbtt_ScaleForPixelHeight(&info, lineHeight);

    int hCharWidth, wCharWidth;
    stbtt_GetCodepointHMetrics(&info, L'a', &hCharWidth, NULL);
    stbtt_GetCodepointHMetrics(&info, L'里', &wCharWidth, NULL);
    hCharWidth = static_cast<int>(static_cast<float>(hCharWidth) * scale);
    wCharWidth = static_cast<int>(static_cast<float>(wCharWidth) * scale);

    if (!imageSize)
    {
        if (!width)
        {
            for (size_t i = 0; i < strlen(text); i++)
            {
                width += (IS_ASCII_CHAR(text[i]) ? hCharWidth : wCharWidth);
                width += static_cast<int>(
                    static_cast<float>(stbtt_GetCodepointKernAdvance(&info, text[i], text[i + 1])) * scale);
            }
        }
        height = static_cast<int>(lineHeight);
        imageSize = static_cast<int>(static_cast<float>(width) * scaleX * lineHeight);
    }

    std::vector<unsigned char> textBitmap(static_cast<size_t>(imageSize));

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent = static_cast<int>(static_cast<float>(ascent) * scale);
    descent = static_cast<int>(static_cast<float>(descent) * scale);
    for (size_t i = 0, x = 0; i < strlen(text); i++)
    {
        int l, t, r, b;
        stbtt_GetCodepointBitmapBox(&info, text[i], scale, scale, &l, &t, &r, &b);

        int y = ascent + t;

        int byteOffset = static_cast<int>(x) + (y * width);
        stbtt_MakeCodepointBitmap(&info, &textBitmap[static_cast<size_t>(byteOffset)], r - l, b - t, width, scale, scale, text[i]);

        //int ax;
        //stbtt_GetCodepointHMetrics(&info, text[i], &ax, 0);
        //x += ax * scale;
        x += static_cast<size_t>(IS_ASCII_CHAR(text[i]) ? hCharWidth : wCharWidth);

        x += static_cast<size_t>(static_cast<float>(stbtt_GetCodepointKernAdvance(&info, text[i], text[i + 1])) * scale);
    }

    VkDeviceSize bufSize = static_cast<VkDeviceSize>(imageSize) * 4;
    Buffer aBuffer(this, bufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    aBuffer.Map();
    //memcpy(aBuffer.Getconst stbtt_fontinfo *infoMappedData(), textBitmap.data(), static_cast<size_t>(imageSize));
    auto bufData = static_cast<unsigned char*>(aBuffer.GetMappedData());
    for (VkDeviceSize i = 0, j = 0; i < bufSize; i += 4, j++)
    {
        if (textBitmap[j] >= 0xC0)
        {
            bufData[i] = textBitmap[j];
            bufData[i + 1] = bufData[i];
            bufData[i + 2] = bufData[i];
            bufData[i + 3] = 0xFF;
        }
        else
        {
            bufData[i] = 0x00;
            bufData[i + 1] = 0x00;
            bufData[i + 2] = 0x00;
            bufData[i + 3] = 0x00;
        }
    }
    aBuffer.Unmap();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    CreateImage(&imageInfo, pTextImage, allocation);

    TransitionImageLayout(*pTextImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(aBuffer.GetBuffer(), *pTextImage, imageInfo.extent);
    TransitionImageLayout(*pTextImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

#endif

void Device::BuildFontVertices(std::vector<Character>& characters, int range)
{
    auto fontData = ReadFile("Fonts/SourceCodePro-Black.otf");
    stbtt_fontinfo info{};
    if (!stbtt_InitFont(&info, const_cast<const unsigned char*>(fontData.data()), 0))
        throw std::runtime_error("failed to init font");

    const float scale = 1.0f / 660.0f;
    characters.resize(static_cast<size_t>(range));
    uint32_t indexCount = 0;
    for (int cid = 0; cid < range; cid++)
    {
        int glyphIndex = stbtt_FindGlyphIndex(&info, cid);
        stbtt_vertex* vertices;
        int vertexCount = stbtt_GetGlyphShape(&info, glyphIndex, &vertices);
        auto& character = characters[static_cast<size_t>(cid)];
        character.paths = {};
        character.indexOffset = indexCount;
        auto& paths = character.paths;
        std::vector<glm::vec2> currentPath{};
        glm::vec2 minBounding{std::numeric_limits<float>::max()};
        glm::vec2 maxBounding{-std::numeric_limits<float>::max()};
        for (int i = 0; i < vertexCount; i++)
        {
            auto& vertex = vertices[i];
            switch(vertex.type)
            {
                case STBTT_vmove:
                    if (!currentPath.empty())
                    {
                        paths.push_back(currentPath);
                        character.indices.push_back(UINT_MAX);
                        currentPath = {};
                    }
                    [[fallthrough]];
                case STBTT_vline:
                    character.indices.push_back(character.vertices.size());
                    currentPath.push_back({static_cast<float>(vertex.x) * scale, static_cast<float>(vertex.y) * scale});
                    character.vertices.push_back(currentPath.back());
                    minBounding = glm::min(minBounding, currentPath.back());
                    maxBounding = glm::max(maxBounding, currentPath.back());
                    break;
                case STBTT_vcurve:
                {
                    glm::vec2 p0 = currentPath.back();
                    glm::vec2 p1 = {static_cast<float>(vertex.cx) * scale, static_cast<float>(vertex.cy) * scale};
                    glm::vec2 p2 = {static_cast<float>(vertex.x)  * scale, static_cast<float>(vertex.y)  * scale};
                    const int step = 5;
                    glm::vec2 t1 = (p1 - p0) / static_cast<float>(step);
                    glm::vec2 t2 = (p2 - p1) / static_cast<float>(step);
                    for (int j = 0; j < step; j++)
                    {
                        glm::vec2 l0 = p0 + t1 * static_cast<float>(j);
                        glm::vec2 l1 = p1 + t2 * static_cast<float>(j);
                        glm::vec2 t3 = (l1 - l0) / static_cast<float>(step);
                        character.indices.push_back(character.vertices.size());
                        currentPath.push_back(l0 + t3 * static_cast<float>(j));
                        character.vertices.push_back(currentPath.back());
                        minBounding = glm::min(minBounding, currentPath.back());
                        maxBounding = glm::max(maxBounding, currentPath.back());
                    }
                    character.indices.push_back(character.vertices.size());
                    currentPath.push_back(p2);
                    character.vertices.push_back(currentPath.back());
                    minBounding = glm::min(minBounding, currentPath.back());
                    maxBounding = glm::max(maxBounding, currentPath.back());
                }
                    break;
                case STBTT_vcubic:
                {
                    glm::vec2 p0 = currentPath.back();
                    glm::vec2 p1 = {vertex.cx * scale, vertex.cy * scale};
                    glm::vec2 p2 = {vertex.cx1 * scale, vertex.cy1 * scale};
                    glm::vec2 p3 = {vertex.x * scale, vertex.y * scale};

                    const int steps = 5;
                    for (int j = 1; j <= steps; ++j)
                    {
                        float t = float(j) / steps;
                        float it = 1.0f - t;
                        glm::vec2 pt = it * it * it * p0 +
                                3 * it * it * t * p1 +
                                3 * it * t * t * p2 +
                                t * t * t * p3;
                        character.indices.push_back(character.vertices.size());
                        currentPath.push_back(pt);
                        character.vertices.push_back(currentPath.back());
                        minBounding = glm::min(minBounding, currentPath.back());
                        maxBounding = glm::max(maxBounding, currentPath.back());
                    }
                }
                    break;
                default:
                    break;
            }
        }
        if (currentPath.size())
        {
            paths.push_back(currentPath);
            character.indices.push_back(UINT_MAX);
        }
        indexCount += uint32_t(character.indices.size());
        character.center = (minBounding + maxBounding) * 0.5f;
        character.height = std::abs(maxBounding.y - minBounding.y);
        character.width = std::abs(maxBounding.x - minBounding.x);
    }
}

void Device::CreateSampler(VkSampler* pSampler, enum VkSamplerAddressMode samplerAddressMode, VkBorderColor borderColor, VkCompareOp compareOp)
{
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = samplerAddressMode;
    samplerInfo.addressModeV = samplerAddressMode;
    samplerInfo.addressModeW = samplerAddressMode;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = borderColor;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = compareOp;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.maxLod = 1.0f;

    if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr, pSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create sampler");
}

void Device::CreateDescriptorPool(uint32_t descriptorSetCount, uint32_t descriptorCount, VkDescriptorPool& descriptorPool, VkDescriptorType descriptorType, VkCommandPoolCreateFlags flags)
{
    VkDescriptorPoolSize poolSizes[1];
    poolSizes[0].type = descriptorType;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(descriptorCount * descriptorSetCount);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = numsof(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = static_cast<uint32_t>(descriptorSetCount);
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | flags;

    if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool!");
}

void Device::CreateDescriptorPool(uint32_t descriptorSetCount, const VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, VkDescriptorPool& descriptorPool, VkCommandPoolCreateFlags flags)
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizeCount;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = descriptorSetCount;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | flags;

    if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool!");
}

void Device::UpdateDescriptorSet(const VkDescriptorBufferInfo& bufferInfo, uint32_t binding, VkDescriptorType descriptorType, VkDescriptorSet descriptorSet)
{
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = descriptorType;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    vkUpdateDescriptorSets(logicalDevice, 1,
        &descriptorWrite, 0, nullptr);
}

void Device::CreateDescriptorSets(Buffer* buffers, VkDeviceSize bufferSize, uint32_t binding, uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorType descriptorType, std::vector<VkDescriptorSet>& descriptorSets)
{
    std::vector<VkDescriptorSetLayout> layouts(descriptorSetCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetCount);
    allocInfo.pSetLayouts = layouts.data();
    descriptorSets.resize(descriptorSetCount);

    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }
    for (uint32_t i = 0; i < descriptorSetCount; i++)
    {
        auto& buffer = buffers[i];
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = bufferSize;
        UpdateDescriptorSet(bufferInfo, binding, descriptorType, descriptorSets[i]);
    }
}

void Device::CreateDescriptorSets(VkDescriptorImageInfo* imageInfos, uint32_t binding, uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorType descriptorType, std::vector<VkDescriptorSet>& descriptorSets)
{
    std::vector<VkDescriptorSetLayout> layouts(descriptorSetCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetCount);
    allocInfo.pSetLayouts = layouts.data();
    descriptorSets.resize(descriptorSetCount);

    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }
    for (uint32_t i = 0; i < descriptorSetCount; i++)
    {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfos[i];
        vkUpdateDescriptorSets(logicalDevice, 1,
            &descriptorWrite, 0, nullptr);
    }
}

void Device::CreateDescriptorSets(uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, std::vector<VkDescriptorSet>& descriptorSets, void* pNext)
{
    std::vector<VkDescriptorSetLayout> layouts(descriptorSetCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = descriptorSetCount;
    allocInfo.pSetLayouts = layouts.data();
    allocInfo.pNext = pNext;
    descriptorSets.resize(descriptorSetCount);
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }
}

void Device::CreateDescriptorSets(uint32_t descriptorSetCount, VkDescriptorPool& descriptorPool,
    VkDescriptorSetLayout& descriptorSetLayout, std::vector<VkDescriptorSet>& descriptorSets,
    std::vector<std::vector<VkWriteDescriptorSet>>& writes)
{
    CreateDescriptorSets(descriptorSetCount, descriptorPool, descriptorSetLayout, descriptorSets, nullptr);
    for (uint32_t i = 0; i < descriptorSetCount; i++)
    {
        for (auto& write : writes[i])
            write.dstSet = descriptorSets[i];
        vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(writes[i].size()),
            writes[i].data(), 0, nullptr);
    }
}

VkDescriptorSetLayoutBinding Device::CreateLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.descriptorCount = descriptorCount;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.pImmutableSamplers = nullptr;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.binding = binding;
    return layoutBinding;
}

std::vector<VkDescriptorSetLayoutBinding> Device::CreateLayoutBindings(uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t descriptorCount)
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.resize(descriptorCount);
    for (auto &layoutBinding : layoutBindings)
    {
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBinding.stageFlags = stageFlags;
        layoutBinding.binding = binding++;
    }
    return layoutBindings;
}

void Device::TransitionImageLayout(VkImage &image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
        );

    endSingleTimeCommands(commandBuffer);
}

void Device::WaitBufferIdle(VkBuffer &buffer)
{
    VkBufferMemoryBarrier bufferBarrier{};
    bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bufferBarrier.dstAccessMask = 0;
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.buffer = buffer;
    bufferBarrier.offset = 0;
    bufferBarrier.size = VK_WHOLE_SIZE;
    auto commandBuffer = beginSingleTimeCommands();
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);
    endSingleTimeCommands(commandBuffer);
}

Device::QueueFamilyIndices Device::GetQueueFamiliesForCurrent()
{
    return FindQueueFamilies(physicalDevice);
}

VkQueue &Device::GetGraphicsQueue()
{
    return graphicsQueue;
}
VkQueue &Device::GetPresentQueue()
{
    return presentQueue;
}

VkSampleCountFlagBits Device::GetMaxUsableSampleCount()
{
    return usableSamples.size() >= 2 ? *(usableSamples.end() - 2) : usableSamples.front();
}

Device::QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        //if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
        {
            indices.graphicsAndComputeFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

Device::SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    if (count != 0)
    {
        details.formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, details.formats.data());
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

    if (count != 0)
    {
        details.presentModes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, details.presentModes.data());
    }

    return details;
}

VkDevice Device::GetLogicalDevice()
{
    return logicalDevice;
}

VkPhysicalDevice Device::GetPhysicalDevice()
{
    return physicalDevice;
}

void Device::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (!deviceCount)
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    int currentScore, bestScore = 0;
    VkPhysicalDeviceMeshShaderPropertiesEXT _meshShaderProperties = {};
    _meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamicStateProperties{};
    dynamicStateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    dynamicStateProperties.pNext = &_meshShaderProperties;

    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &dynamicStateProperties;
    for (const auto &device : devices)
    {
        currentScore = 0;
        if (isDeviceSuitable(device))
        {
            currentScore = 30;
            vkGetPhysicalDeviceProperties2(device, &deviceProperties2);
            currentScore += deviceProperties2.properties.limits.framebufferColorSampleCounts & deviceProperties2.properties.limits.framebufferDepthSampleCounts;
            currentScore += deviceProperties2.properties.limits.storageImageSampleCounts + deviceProperties2.properties.limits.maxColorAttachments;
            std::string deviceName = physicalDeviceProperties.deviceName;
            if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                currentScore += 10;
            if (deviceName.find("AMD") != -1llu)
            {
                currentScore += 3;
                if (deviceName.find("RX") != -1llu)
                    currentScore += 4;
            }
            else if (deviceName.find("NVIDIA") != -1llu)
            {
                currentScore += 3;
                if (deviceName.find("RTX") != -1llu)
                    currentScore += 4;
                else if (deviceName.find("GTX") != -1llu)
                    currentScore += 1;
            }
            if (currentScore > bestScore)
            {
                physicalDevice = device;
                physicalDeviceProperties = deviceProperties2.properties;
                meshShaderProperties = _meshShaderProperties;
                bestScore = currentScore;
            }
        }
    }
    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU!");

    checkUsableSamples();
}

void Device::checkUsableSamples()
{
    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) {  usableSamples.push_back(VK_SAMPLE_COUNT_64_BIT); }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { usableSamples.push_back(VK_SAMPLE_COUNT_32_BIT); }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { usableSamples.push_back(VK_SAMPLE_COUNT_16_BIT); }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { usableSamples.push_back(VK_SAMPLE_COUNT_8_BIT); }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { usableSamples.push_back(VK_SAMPLE_COUNT_4_BIT); }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { usableSamples.push_back(VK_SAMPLE_COUNT_2_BIT); }

    usableSamples.push_back(VK_SAMPLE_COUNT_1_BIT);
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

#ifdef ENABLE_MESH_SHADER
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;

    VkPhysicalDeviceFeatures2 deviceFeatures = {};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.pNext = &meshShaderFeatures;
    vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);
    if ((meshShaderSupport = meshShaderFeatures.meshShader == VK_TRUE))
    {
        deviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }
#endif
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Device::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool extSupported = checkDeviceExtensionSupport(device), swapChainAdequate = false;

    if (extSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() &&!swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void Device::createLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f; // 0.0~1.0
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT primitiveRestartFeatures{};
    primitiveRestartFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
    primitiveRestartFeatures.primitiveTopologyListRestart = VK_TRUE;

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features{};
    dynamicState3Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    dynamicState3Features.pNext = &primitiveRestartFeatures;

    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.drawIndirectCount = VK_TRUE;
    vulkan12Features.descriptorIndexing = VK_TRUE;
    vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
    vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
    vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    vulkan12Features.runtimeDescriptorArray = VK_TRUE;
    vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vulkan12Features.pNext = &dynamicState3Features;

    VkPhysicalDeviceNestedCommandBufferFeaturesEXT nestedCommandBufferFeatures{};
    nestedCommandBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT;
    nestedCommandBufferFeatures.nestedCommandBufferSimultaneousUse = VK_TRUE;
    nestedCommandBufferFeatures.nestedCommandBuffer = VK_TRUE;
    nestedCommandBufferFeatures.pNext = &vulkan12Features;
    VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParametersFeatures{};
    shaderDrawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shaderDrawParametersFeatures.shaderDrawParameters = VK_TRUE;
    shaderDrawParametersFeatures.pNext = &nestedCommandBufferFeatures;
    //VkPhysicalDeviceFeatures deviceFeatures1{};
    //vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures1);

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    //vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
    deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
    deviceFeatures2.features.sampleRateShading = VK_TRUE;
    deviceFeatures2.features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
    deviceFeatures2.features.vertexPipelineStoresAndAtomics = VK_TRUE;
#ifdef ENABLE_MESH_SHADER
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.meshShader = VK_TRUE;
    meshShaderFeatures.taskShader = VK_TRUE;
    meshShaderFeatures.pNext = &shaderDrawParametersFeatures;
    if (meshShaderSupport)
        deviceFeatures2.pNext = &meshShaderFeatures;
    else
        deviceFeatures2.pNext = &shaderDrawParametersFeatures;

#else
    deviceFeatures2.pNext = &shaderDrawParametersFeatures;
#endif

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    //createInfo.pEnabledFeatures = &deviceFeatures2.features;
    createInfo.pNext = &deviceFeatures2;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(logicalDevice, "vkCmdDrawMeshTasksEXT"));
    vkCmdDrawMeshTasksIndirectCountEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(logicalDevice, "vkCmdDrawMeshTasksIndirectCountEXT"));
    vkGetDeviceQueue(logicalDevice, indices.graphicsAndComputeFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
      if ((typeFilter  &(1 << i)) &&
          (memProperties.memoryTypes[i].propertyFlags  &properties) ==
              properties)
        {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

void Device::CreateCommandPool(VkCommandPoolCreateFlags flags, VkCommandPool* pool)
{
    Device::QueueFamilyIndices queueFamilyIndices = GetQueueFamiliesForCurrent();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flags;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

    if (vkCreateCommandPool(logicalDevice, &poolInfo,
    nullptr, pool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");
}

VkCommandBuffer Device::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

VmaAllocator Device::GetAllocator()
{
    return allocator;
}

void Device::createVmaAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = logicalDevice;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS)
        throw std::runtime_error("failed to create vma allocator");
}
