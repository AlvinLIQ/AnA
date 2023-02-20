#include "Headers/AnA_Model.h"
#include "Headers/AnA_Buffer.h"
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstring>
#include <iostream>


using namespace AnA;

AnA_Model::AnA_Model(AnA_Device *&mDevice, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices) : aDevice{mDevice}
{
    createVertexBuffers(vertices);
    createIndexBuffers(indices);
}

AnA_Model::~AnA_Model()
{
    delete indexBuffer;
    delete vertexBuffer;
}

void AnA_Model::createVertexBuffers(const std::vector<Vertex> &vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    vertexBuffer = new AnA_Buffer(aDevice, bufferSize, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);//Host CPU Device GPU

    vertexBuffer->CopyToBuffer(vertices.data(), bufferSize);
}

void AnA_Model::createIndexBuffers(const std::vector<uint16_t> &indices)
{
    indexCount = static_cast<uint32_t>(indices.size());
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
    indexBuffer = new AnA_Buffer(aDevice, bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    indexBuffer->CopyToBuffer(indices.data(), bufferSize);
}

void AnA_Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {vertexBuffer->GetBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
}

void AnA_Model::Draw(VkCommandBuffer commandBuffer)
{
    //vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    for (int i = 0; i < vertexCount; i += 4)
    {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, i, 0);
    }
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