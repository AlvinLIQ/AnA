#include "Headers/Device.hpp"

#include <mutex>
#include <set>
#include <stdexcept>
//#include <chrono>

#include "Headers/Buffer.hpp"
#include "vulkan/vulkan_core.h"

#ifdef INCLUDE_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "../3rdParty/stb/stb_image.h"
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "../3rdParty/stb/stb_truetype.h"

#include "CDT.h"

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
    createSubCommandResources();
}

Device::~Device()
{
    vkDestroyCommandPool(logicalDevice, subCommandPool, nullptr);
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
    if (deviceFeatures.bufferDeviceAddressSupport)
        bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
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

void Device::HostImageLayoutTransition(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkHostImageLayoutTransitionInfo transitionInfo{};
    transitionInfo.sType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO;
    transitionInfo.image = image;
    transitionInfo.oldLayout = oldLayout;
    transitionInfo.newLayout = newLayout;
    transitionInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    transitionInfo.subresourceRange.layerCount = 1;
    transitionInfo.subresourceRange.levelCount = 1;

    vkTransitionImageLayout(logicalDevice, 1, &transitionInfo);
}

void Device::CopyHostBufferToImage(void* buffer, VkImage dstImage, VkExtent3D extent)
{
    VkMemoryToImageCopyEXT region{};

    region.sType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY;
    region.pHostPointer = buffer;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = extent;

    VkCopyMemoryToImageInfo copyInfo{};
    copyInfo.sType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO;
    copyInfo.dstImage = dstImage;
    copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    copyInfo.regionCount = 1;
    copyInfo.pRegions = &region;

    vkCopyMemoryToImage(logicalDevice, &copyInfo);
}

void Device::CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage& dstImage, VkExtent3D extent)
{
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

void Device::CopyBufferToImage(Buffer* stagingBuffer, VkImage* pTexImage, VkExtent3D& extent)
{
    VkCommandBuffer commandBuffer = BeginSubCommands();
    TransitionImageLayout(commandBuffer,
        *pTexImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(commandBuffer, stagingBuffer->GetBuffer(), *pTexImage, extent);
    TransitionImageLayout(commandBuffer,
        *pTexImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    stagingBuffers.push_back(stagingBuffer);
    EndSubCommands();
}

void Device::CreateColorImage(const uint32_t color, VkImage* pTexImage, VmaAllocation& allocation)
{
    //auto p = std::chrono::high_resolution_clock::now();
    Buffer* aBuffer = new Buffer(this, sizeof(color), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    aBuffer->Map();
    memcpy(aBuffer->GetMappedData(), &color, sizeof(color));

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
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (deviceFeatures.hostImageCopySupport)
        imageInfo.usage |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
    else
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    CreateImage(&imageInfo, pTexImage, allocation);

    if (deviceFeatures.hostImageCopySupport)
    {
        HostImageLayoutTransition(*pTexImage, imageInfo.initialLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        CopyHostBufferToImage(aBuffer->GetMappedData(), *pTexImage, imageInfo.extent);
        delete aBuffer;
    }
    else
    {
        CopyBufferToImage(aBuffer, pTexImage, imageInfo.extent);
    }
    /*
    auto c = std::chrono::high_resolution_clock::now();

    auto transTime = std::chrono::duration<float, std::chrono::seconds::period>(c - p).count();
    printf("%f\n", transTime);*/
}

#ifdef INCLUDE_STB_IMAGE
void Device::CreateTextureImage(const char* imagePath, VkImage* pTexImage, VmaAllocation& allocation)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(imagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

    if (!pixels)
        throw std::runtime_error(std::string("Failed to load texture image! ") + imagePath);

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
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;
    if (deviceFeatures.hostImageCopySupport)
    {
        imageInfo.usage = VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        CreateImage(&imageInfo, pTexImage, allocation);
        HostImageLayoutTransition(*pTexImage, imageInfo.initialLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        CopyHostBufferToImage(pixels, *pTexImage, imageInfo.extent);
        stbi_image_free(pixels);
    }
    else
    {
        Buffer* aBuffer = new Buffer(this, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
        aBuffer->Map();
        memcpy(aBuffer->GetMappedData(), pixels, static_cast<size_t>(imageSize));
        aBuffer->Unmap();
        stbi_image_free(pixels);

        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        CreateImage(&imageInfo, pTexImage, allocation);

        CopyBufferToImage(aBuffer, pTexImage, imageInfo.extent);
    }
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
    Buffer* aBuffer = new Buffer(this, bufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    aBuffer->Map();
    //memcpy(aBuffer.Getconst stbtt_fontinfo *infoMappedData(), textBitmap.data(), static_cast<size_t>(imageSize));
    auto bufData = static_cast<unsigned char*>(aBuffer->GetMappedData());
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
    aBuffer->Unmap();

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

    CopyBufferToImage(aBuffer, pTextImage, imageInfo.extent);
}

#endif

void Device::BuildFontVertices(std::unordered_map<int, Character>& characters, int offset, int range)
{
    auto fontData = ReadFile("Fonts/SourceCodePro-Black.otf");
    stbtt_fontinfo info{};
    if (!stbtt_InitFont(&info, const_cast<const unsigned char*>(fontData.data()), 0))
        throw std::runtime_error("failed to init font");

    const float scale = 1.0f / 660.0f;
    uint32_t indexCount = 0;
    for (int cid = offset; cid < range; cid++)
    {
        int glyphIndex = stbtt_FindGlyphIndex(&info, cid);
        stbtt_vertex* vertices;
        int vertexCount = stbtt_GetGlyphShape(&info, glyphIndex, &vertices);
        auto& character = characters.try_emplace(cid).first->second;
        character.paths = {};
        character.indexOffset = indexCount;
        auto& paths = character.paths;
        std::vector<glm::vec2> currentPath{};
        glm::vec2 minBounding{std::numeric_limits<float>::max()};
        glm::vec2 maxBounding{-std::numeric_limits<float>::max()};
        CDT::Triangulation<float> cdt;
        std::vector<CDT::Edge> edges{};
        std::unordered_map<CDT::V2d<float>, uint32_t> vertexMap{};
        uint32_t lastIndex = 0;
        for (int i = 0; i < vertexCount; i++)
        {
            auto& vertex = vertices[i];
            switch(vertex.type)
            {
                case STBTT_vmove:
                    if (!currentPath.empty())
                    {
                        paths.push_back(currentPath);
                        currentPath = {};
                    }
                    [[fallthrough]];
                case STBTT_vline:
                {
                    currentPath.push_back({static_cast<float>(vertex.x) * scale, static_cast<float>(vertex.y) * scale});
                    auto iter = vertexMap.find(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()));
                    if (iter == vertexMap.end())
                    {
                        iter = vertexMap.emplace(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()), character.vertices.size()).first;
                        character.vertices.push_back(currentPath.back());
                    }
                    if (currentPath.size() > 1)
                    {
                        edges.push_back({lastIndex, iter->second});
                    }
                    lastIndex = iter->second;
                }
                    minBounding = glm::min(minBounding, currentPath.back());
                    maxBounding = glm::max(maxBounding, currentPath.back());
                    break;
                case STBTT_vcurve:
                {
                    glm::vec2 p0 = currentPath.back();
                    glm::vec2 p1 = {static_cast<float>(vertex.cx) * scale, static_cast<float>(vertex.cy) * scale};
                    glm::vec2 p2 = {static_cast<float>(vertex.x)  * scale, static_cast<float>(vertex.y)  * scale};
                    const int step = 3;
                    glm::vec2 t1 = (p1 - p0) / static_cast<float>(step);
                    glm::vec2 t2 = (p2 - p1) / static_cast<float>(step);
                    for (int j = 0; j < step; j++)
                    {
                        glm::vec2 l0 = p0 + t1 * static_cast<float>(j);
                        glm::vec2 l1 = p1 + t2 * static_cast<float>(j);
                        glm::vec2 t3 = (l1 - l0) / static_cast<float>(step);
                        currentPath.push_back(l0 + t3 * static_cast<float>(j));
                        auto iter = vertexMap.find(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()));
                        if (iter == vertexMap.end())
                        {
                            iter = vertexMap.emplace(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()), character.vertices.size()).first;
                            character.vertices.push_back(currentPath.back());
                        }
                        if (currentPath.size() > 1)
                        {
                            edges.push_back({lastIndex, iter->second});
                        }
                        lastIndex = iter->second;
                        minBounding = glm::min(minBounding, currentPath.back());
                        maxBounding = glm::max(maxBounding, currentPath.back());
                    }
                    currentPath.push_back(p2);
                    auto iter = vertexMap.find(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()));
                    if (iter == vertexMap.end())
                    {
                        iter = vertexMap.emplace(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()), character.vertices.size()).first;
                        character.vertices.push_back(currentPath.back());
                    }
                    if (currentPath.size() > 1)
                    {
                        edges.push_back({lastIndex, iter->second});
                    }
                    lastIndex = iter->second;
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

                    const int steps = 3;
                    for (int j = 1; j <= steps; ++j)
                    {
                        float t = float(j) / steps;
                        float it = 1.0f - t;
                        glm::vec2 pt = it * it * it * p0 +
                                3 * it * it * t * p1 +
                                3 * it * t * t * p2 +
                                t * t * t * p3;
                        currentPath.push_back(pt);
                        auto iter = vertexMap.find(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()));
                        if (iter == vertexMap.end())
                        {
                            iter = vertexMap.emplace(*reinterpret_cast<CDT::V2d<float>*>(&currentPath.back()), character.vertices.size()).first;
                            character.vertices.push_back(currentPath.back());
                        }
                        if (currentPath.size() > 1)
                        {
                            edges.push_back({lastIndex, iter->second});
                        }
                        lastIndex = iter->second;
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
        }
        CDT::RemoveDuplicatesAndRemapEdges(*reinterpret_cast<std::vector<CDT::V2d<float>>*>(&character.vertices), edges);
        cdt.insertVertices(*reinterpret_cast<std::vector<CDT::V2d<float>>*>(&character.vertices));
        cdt.insertEdges(edges);
        cdt.eraseOuterTrianglesAndHoles();
        for (auto& triangle : cdt.triangles)
        {
            character.indices.push_back(triangle.vertices[0]);
            character.indices.push_back(triangle.vertices[1]);
            character.indices.push_back(triangle.vertices[2]);
        }
        indexCount += uint32_t(character.indices.size());
        character.center = (minBounding + maxBounding) * 0.5f;
        character.height = std::abs(maxBounding.y - minBounding.y);
        character.width = std::abs(maxBounding.x - minBounding.x);
    }
}

void Device::Triangulation(std::vector<glm::vec2>& vertices, std::vector<glm::uvec2>& edges, std::vector<uint32_t>& indices)
{
    CDT::Triangulation<float> cdt;

    CDT::RemoveDuplicatesAndRemapEdges(*reinterpret_cast<std::vector<CDT::V2d<float>>*>(&vertices), *reinterpret_cast<std::vector<CDT::Edge>*>(&edges));
    cdt.insertVertices(*reinterpret_cast<std::vector<CDT::V2d<float>>*>(&vertices));
    cdt.insertEdges(*reinterpret_cast<std::vector<CDT::Edge>*>(&edges));
    cdt.eraseOuterTriangles();

    vertices = *reinterpret_cast<std::vector<glm::vec2>*>(&cdt.vertices);

    for (auto& triangle : cdt.triangles)
    {
        indices.push_back(triangle.vertices[0]);
        indices.push_back(triangle.vertices[1]);
        indices.push_back(triangle.vertices[2]);
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
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr, pSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create sampler");
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

void Device::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage &image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
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
}

Device::QueueFamilyIndices Device::GetQueueFamiliesForCurrent()
{
    return queueFamilyIndices;
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
    return usableSamples.size() > 3 ? usableSamples[2] : usableSamples.front();
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
        if (!indices.graphicsAndComputeFamily.has_value()
            && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
        {
            indices.graphicsAndComputeFamily = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {

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
    VkPhysicalDeviceDescriptorBufferPropertiesEXT _descriptorBufferProperties{};
    _descriptorBufferProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;

    VkPhysicalDeviceMeshShaderPropertiesEXT _meshShaderProperties{};
    _meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
    _meshShaderProperties.pNext = &_descriptorBufferProperties;

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamicStateProperties{};
    dynamicStateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    dynamicStateProperties.pNext = &_meshShaderProperties;

    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &dynamicStateProperties;

    QueueFamilyIndices indices;

    for (const auto &device : devices)
    {
        currentScore = 0;
        DeviceFeatures _deviceFeatures{};
        if (isDeviceSuitable(device, _deviceFeatures, indices))
        {
            currentScore = 30;
            vkGetPhysicalDeviceProperties2(device, &deviceProperties2);
            currentScore += deviceProperties2.properties.limits.framebufferColorSampleCounts & deviceProperties2.properties.limits.framebufferDepthSampleCounts;
            currentScore += deviceProperties2.properties.limits.storageImageSampleCounts + deviceProperties2.properties.limits.maxColorAttachments;
            std::string deviceName = deviceProperties2.properties.deviceName;
            if (deviceProperties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                currentScore += 10;
            if (deviceName.find("AMD") != size_t(-1))
            {
                currentScore += 3;
                if (deviceName.find("RX") != size_t(-1))
                    currentScore += 4;
            }
            else if (deviceName.find("NVIDIA") != size_t(-1))
            {
                currentScore += 3;
                if (deviceName.find("RTX") != size_t(-1))
                    currentScore += 4;
                else if (deviceName.find("GTX") != size_t(-1))
                    currentScore += 1;
            }
            if (currentScore > bestScore)
            {
                deviceFeatures = _deviceFeatures;
                physicalDevice = device;
                physicalDeviceProperties = deviceProperties2.properties;
                meshShaderProperties = _meshShaderProperties;
                descriptorBufferProperties = _descriptorBufferProperties;
                queueFamilyIndices = indices;
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

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device, DeviceFeatures& _deviceFeatures)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
    bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;

    VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedLayoutsFeatures{};
    unifiedLayoutsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR;
    unifiedLayoutsFeatures.pNext = &bufferDeviceAddressFeatures;

    VkPhysicalDeviceHostImageCopyFeaturesEXT hostImageCopyFeatures = {};
    hostImageCopyFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT;
    hostImageCopyFeatures.pNext = &unifiedLayoutsFeatures;

#ifdef ENABLE_MESH_SHADER
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    unifiedLayoutsFeatures.pNext = &meshShaderFeatures;
#endif

    VkPhysicalDeviceFeatures2 deviceFeatures = {};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.pNext = &hostImageCopyFeatures;
    vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

#ifdef ENABLE_MESH_SHADER
    if (meshShaderFeatures.meshShader == VK_TRUE)
    {
        _deviceFeatures.meshShaderSupport = true;
        deviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }
#endif

    _deviceFeatures.unifiedLayoutsSupport = unifiedLayoutsFeatures.unifiedImageLayouts == VK_TRUE;
    _deviceFeatures.hostImageCopySupport = hostImageCopyFeatures.hostImageCopy == VK_TRUE;
    _deviceFeatures.bufferDeviceAddressSupport = true;//bufferDeviceAddressFeatures.bufferDeviceAddress == VK_TRUE;

    return requiredExtensions.empty();
}

bool Device::isDeviceSuitable(VkPhysicalDevice device, DeviceFeatures& _deviceFeatures, QueueFamilyIndices& indices)
{
    indices = FindQueueFamilies(device);

    bool extSupported = checkDeviceExtensionSupport(device, _deviceFeatures), swapChainAdequate = false;

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
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsAndComputeFamily.value(), queueFamilyIndices.presentFamily.value()};

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
    VkPhysicalDeviceVulkan14Features vulkan14Features{};
    vulkan14Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    vulkan14Features.hostImageCopy = deviceFeatures.hostImageCopySupport;
    //vulkan14Features.dynamicRenderingLocalRead = VK_TRUE;

    VkPhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13Features.maintenance4 = VK_TRUE;
    vulkan13Features.synchronization2 = VK_TRUE;
    vulkan13Features.dynamicRendering = VK_TRUE;
    vulkan13Features.shaderDemoteToHelperInvocation = VK_TRUE;
    vulkan13Features.pNext = &vulkan14Features;

    VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures{};
    descriptorBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    descriptorBufferFeatures.descriptorBuffer = VK_TRUE;
    descriptorBufferFeatures.pNext = &vulkan13Features;

    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT primitiveRestartFeatures{};
    primitiveRestartFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
    primitiveRestartFeatures.primitiveTopologyListRestart = VK_TRUE;
    primitiveRestartFeatures.pNext = &descriptorBufferFeatures;

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features{};
    dynamicState3Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
    dynamicState3Features.extendedDynamicState3PolygonMode = VK_TRUE;
    dynamicState3Features.pNext = &primitiveRestartFeatures;

    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.drawIndirectCount = VK_TRUE;
    vulkan12Features.descriptorIndexing = VK_TRUE;
    vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
    vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
    vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    vulkan12Features.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    vulkan12Features.runtimeDescriptorArray = VK_TRUE;
    vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vulkan12Features.bufferDeviceAddress = deviceFeatures.bufferDeviceAddressSupport;
    vulkan12Features.scalarBlockLayout = VK_TRUE;
    vulkan12Features.shaderInt8 = VK_TRUE;
    vulkan12Features.storageBuffer8BitAccess = VK_TRUE;
    vulkan12Features.pNext = &dynamicState3Features;

    VkPhysicalDeviceVulkan11Features vulkan11Features{};
    vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vulkan11Features.storageBuffer16BitAccess = VK_TRUE;
    vulkan11Features.shaderDrawParameters = VK_TRUE;
    vulkan11Features.pNext = &vulkan12Features;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    //vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
    deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
    deviceFeatures2.features.sampleRateShading = VK_TRUE;
    deviceFeatures2.features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
    deviceFeatures2.features.vertexPipelineStoresAndAtomics = VK_TRUE;
    deviceFeatures2.features.fillModeNonSolid = VK_TRUE;
    deviceFeatures2.features.fragmentStoresAndAtomics = VK_TRUE;
    deviceFeatures2.features.shaderInt16 = VK_TRUE;
    deviceFeatures2.features.shaderInt64 = VK_TRUE;
#ifdef ENABLE_MESH_SHADER
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.meshShader = VK_TRUE;
    meshShaderFeatures.taskShader = VK_TRUE;
    meshShaderFeatures.pNext = &vulkan11Features;
    if (deviceFeatures.meshShaderSupport)
        deviceFeatures2.pNext = &meshShaderFeatures;
    else
        deviceFeatures2.pNext = &vulkan11Features;

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
/*
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;*/

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");
    volkLoadDevice(logicalDevice);

    vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(logicalDevice, "vkCmdBeginRenderingKHR"));
    vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetDeviceProcAddr(logicalDevice, "vkCmdEndRenderingKHR"));
    vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(logicalDevice, "vkCmdDrawMeshTasksEXT"));
    vkCmdDrawMeshTasksIndirectCountEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(logicalDevice, "vkCmdDrawMeshTasksIndirectCountEXT"));
    vkCmdSetPolygonModeEXT = reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(vkGetDeviceProcAddr(logicalDevice, "vkCmdSetPolygonModeEXT"));
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsAndComputeFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
}

void Device::createSubCommandResources()
{
    CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &subCommandPool);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = subCommandPool;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &subCommandBuffer);
}

void Device::cleanupStagingBuffers()
{
    for (auto stagingBuffer : stagingBuffers)
        delete stagingBuffer;
    stagingBuffers.clear();
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

VkImageMemoryBarrier2 Device::ImageMemoryBarrier2(
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkAccessFlags2 srcAccessMask,
    VkAccessFlags2 dstAccessMask,
    VkPipelineStageFlags2 srcStageMask,
    VkPipelineStageFlags2 dstStageMask,
    VkImageAspectFlags aspectMask)
{
    VkImageMemoryBarrier2 imageBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    return imageBarrier;
}

void Device::PipelineBarrier2(VkCommandBuffer commandBuffer, VkDependencyFlags dependencyFlags,
    VkImageMemoryBarrier2* imageBarriers, uint32_t imageBarrierCount,
    VkBufferMemoryBarrier2* bufferBarriers, uint32_t bufferBarrierCount)
{
    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.dependencyFlags = dependencyFlags;
    depInfo.pImageMemoryBarriers = imageBarriers;
    depInfo.imageMemoryBarrierCount = imageBarrierCount;
    depInfo.pBufferMemoryBarriers = bufferBarriers;
    depInfo.bufferMemoryBarrierCount = bufferBarrierCount;
    vkCmdPipelineBarrier2(commandBuffer, &depInfo);
}

void Device::StageBarrier(VkCommandBuffer commandBuffer,
    VkAccessFlags2 srcAccessMask,
    VkAccessFlags2 dstAccessMask,
    VkPipelineStageFlags2 srcStageMask,
    VkPipelineStageFlags2 dstStageMask)
{
    VkMemoryBarrier2 memBarrier{};
    memBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    memBarrier.srcAccessMask = srcAccessMask;
    memBarrier.dstAccessMask = dstAccessMask;
    memBarrier.srcStageMask = srcStageMask;
    memBarrier.dstStageMask = dstStageMask;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    depInfo.memoryBarrierCount = 1;
    depInfo.pMemoryBarriers = &memBarrier;
    vkCmdPipelineBarrier2(commandBuffer, &depInfo);
}

void Device::StageBarrier(VkCommandBuffer commandBuffer,
    VkPipelineStageFlags2 srcStageMask,
    VkPipelineStageFlags2 dstStageMask)
{
    StageBarrier(commandBuffer,
        VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
        VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
        srcStageMask, dstStageMask);
}

void Device::StageBarrier(VkCommandBuffer commandBuffer,
    VkPipelineStageFlags2 stageMask)
{
    StageBarrier(commandBuffer, stageMask, stageMask);
}


void Device::ImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image,
    VkImageLayout initLayout, VkImageLayout finalLayout,
    VkAccessFlags srcAccessMask, VkImageAspectFlags aspectMask,
    VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcAccessMask = srcAccessMask;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;            // After present
    imageMemoryBarrier.oldLayout = initLayout;
    imageMemoryBarrier.newLayout = finalLayout;          // Target layout for presentation
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image;  // Image to transition
    imageMemoryBarrier.subresourceRange = {};
    imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;

    // Record the barrier in the command buffer
    vkCmdPipelineBarrier(commandBuffer,
        srcStageMask, // Stage before transition
        dstStageMask,            // Stage after transition
        0,                                           // Flags (none)
        0, nullptr,                                  // No memory barriers
        0, nullptr,                                   // No buffer barriers
        1, &imageMemoryBarrier
    );
}

VkCommandBuffer Device::BeginSingleTimeCommands()
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

void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
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

VkCommandBuffer Device::BeginSubCommands()
{
    subCommandMutex.lock();
    if (!subCommandBufferBegan)
    {
        subCommandBufferBegan = true;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(subCommandBuffer, &beginInfo);
    }

    return subCommandBuffer;
}

void Device::EndSubCommands()
{
    subCommandBufferRecorded = true;
    subCommandMutex.unlock();
}

void Device::EndSubCommands(std::function<void()> postProcess)
{
    subCommandPostProcesses.push_back(postProcess);
    EndSubCommands();
}

bool Device::SingleTimeCommandsRecorded()
{
    return subCommandBufferRecorded;
}

bool Device::SingleTimeCommandsSubmitBegan()
{
    return subCommandBufferSubmitBegan;
}

VkCommandBuffer Device::BeginSingleTimeCommandsSubmit()
{
    subCommandMutex.lock();
    if (subCommandBufferBegan)
    {
        vkEndCommandBuffer(subCommandBuffer);
        subCommandBufferBegan = false;
    }
    subCommandBufferSubmitBegan = true;

    return subCommandBuffer;
}

void Device::EndSingleTimeCommandsSubmit(VkFence& fence)
{
    subCommandBufferRecorded = false;
    subCommandBufferSubmitBegan = false;

    vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);
    if (stagingBuffers.size())
        cleanupStagingBuffers();
    for (auto& postProcess : subCommandPostProcesses)
    {
        postProcess();
    }
    subCommandPostProcesses.clear();
    subCommandMutex.unlock();
}

VmaAllocator Device::GetAllocator()
{
    return allocator;
}

VkShaderModule Device::CreateShaderModule(const std::vector<unsigned char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

void Device::createVmaAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    if (deviceFeatures.bufferDeviceAddressSupport)
        allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = logicalDevice;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS)
        throw std::runtime_error("failed to create vma allocator");
}
