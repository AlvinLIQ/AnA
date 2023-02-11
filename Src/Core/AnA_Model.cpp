#include "Headers/AnA_Model.h"
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstring>

using namespace AnA;

AnA_Model::AnA_Model(AnA_Device *&mDevice, const std::vector<Vertex> &vertices) : aDevice{mDevice}
{
    createVertexBuffers(vertices);
}

AnA_Model::~AnA_Model()
{
    vkDestroyBuffer(aDevice->GetLogicalDevice(), vertexBuffer, nullptr);
    vkFreeMemory(aDevice->GetLogicalDevice(), vertexBufferMemory, nullptr);
}

void AnA_Model::createVertexBuffers(const std::vector<Vertex> &vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    aDevice->CreateBuffer(bufferSize, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer,
        vertexBufferMemory);//Host CPU Device GPU

    void* data;
    vkMapMemory(aDevice->GetLogicalDevice(), vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(aDevice->GetLogicalDevice(), vertexBufferMemory);
}

void AnA_Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void AnA_Model::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> AnA_Model::Vertex::GetBindingDescription()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> AnA_Model::Vertex::GetAttributeDescription()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}