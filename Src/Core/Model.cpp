#include "Headers/Model.hpp"
#include "Headers/Buffer.hpp"
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../tinyobjloader/tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>


using namespace AnA;

Model::Model(Device& mDevice, const ModelInfo& modelInfo) : aDevice{mDevice}
{
    createVertexBuffers(modelInfo.vertices);
    if ((hasIndexBuffer = modelInfo.indices.size() > 0))
    {
        createIndexBuffers(modelInfo.indices);
        indexStep = modelInfo.indexStep;
    }
}

Model::~Model()
{
    if (hasIndexBuffer)
        delete indexBuffer;

    delete vertexBuffer;
}

void Model::CreateModelFromFile(Device &mDevice, const char *filePath, std::shared_ptr<Model>& model)
{
    ModelInfo modelInfo{};
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib,& shapes,& materials,& warn,& err, filePath, "Models/"))
        throw std::runtime_error(warn + err);

    modelInfo.vertices.clear();
    modelInfo.indices.clear();

    std::unordered_map<Vertex, Index, VertexHash> verticesMap;
    for (const auto& shape : shapes)
    {
        for (int i = 0; i < shape.mesh.indices.size(); i++)
        {
            const auto& index = shape.mesh.indices[i];
            Vertex vertex{};

            if (index.vertex_index >= 0)
            {
                vertex.position =
                {
                    attrib.vertices[3 * index.vertex_index],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                
                auto colorIndex = 3 * index.vertex_index + 2;
                if (colorIndex < attrib.colors.size())
                {
                    vertex.color = 
                    {
                        attrib.colors[colorIndex - 2],
                        attrib.colors[colorIndex - 1],
                        attrib.colors[colorIndex],
                    };
                }
                if (materials.size())
                {
                    vertex.color = 
                    {
                        materials[0].diffuse[0],
                        materials[0].diffuse[1],
                        materials[0].diffuse[2]
                    };
                }
            }
            if (index.normal_index >= 0)
            {
                vertex.normal =
                {
                    attrib.normals[3 * index.normal_index],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            if (index.texcoord_index >= 0)
            {
                vertex.uv =
                {
                    attrib.texcoords[2 * index.texcoord_index],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }
            //if (index.normal_index + index.vertex_index + index.texcoord_index >= 0)
            auto result = verticesMap.find(vertex);
            if (result != verticesMap.end())
            {
                modelInfo.indices.push_back(result->second);
            }
            else
            {
                verticesMap.insert(std::pair<Vertex, Index>(vertex, modelInfo.vertices.size()));
                modelInfo.indices.push_back(static_cast<Index>(modelInfo.vertices.size()));
                modelInfo.vertices.push_back(vertex);
            }
        }
    }
    const glm::vec<2, int> sets[] = {{0, 1}, {0, 2}, {1, 2}};
    for (int i = 0, j, k = 0; i < modelInfo.vertices.size(); i += k)
    {
        for (k = 0; k < 3; k++)
        {
            glm::vec2 currentProjection{modelInfo.vertices[i + sets[k].x].position.x, modelInfo.vertices[i + sets[k].x].position.x};
            glm::vec2 currentPlane = glm::vec2(modelInfo.vertices[i +  + sets[k].y].position - modelInfo.vertices[i + sets[k].x].position);
            glm::vec2 yBase = glm::mat2(glm::vec2(.0, -1.0), glm::vec2(1.0, 0.0)) * currentPlane;
            glm::mat3 transform{glm::vec3(currentPlane.x, currentPlane.y, 0.0), glm::vec3(yBase.x, yBase.y, 0.0), {}};
            //1.0
            //glm::length(currentPlane);
            for (j = 1; j < modelInfo.vertices.size(); j++)
            {
                if (modelInfo.vertices[j].position.x < currentProjection.x)
                {
                    currentProjection[0] = modelInfo.vertices[j].position.x;
                }
                else if (modelInfo.vertices[j].position.x > currentProjection.y)
                {
                    currentProjection[1] = modelInfo.vertices[j].position.x;
                }
            }

            modelInfo.vertexProjections.push_back(currentProjection);
        }
    }
    model = std::make_shared<Model>(mDevice, modelInfo);
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    Buffer stagingBuffer(aDevice, bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);//Host CPU Device GPU
    stagingBuffer.CopyToBuffer(vertices.data(), bufferSize);

    vertexBuffer = new Buffer(aDevice, bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vertexBuffer->CopyToBuffer(stagingBuffer, bufferSize);
}

void Model::createIndexBuffers(const std::vector<Index>& indices)
{
    indexCount = static_cast<uint32_t>(indices.size());
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
    Buffer stagingBuffer(aDevice, bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.CopyToBuffer(indices.data(), bufferSize);

    indexBuffer = new Buffer(aDevice, bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    indexBuffer->CopyToBuffer(stagingBuffer, bufferSize);
}

void Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {vertexBuffer->GetBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if (hasIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Model::Draw(VkCommandBuffer commandBuffer, Index instanceIndex)
{
    if (hasIndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, instanceIndex);
    }
    else
    {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, instanceIndex);
    }
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescription()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescription()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

    return attributeDescriptions;
}

VkDescriptorSetLayoutBinding Model::ModelStorageBufferObject::GetBindingDescriptionSet()
{
    VkDescriptorSetLayoutBinding ssboLayoutBinding{};
    ssboLayoutBinding.binding = 0;
    ssboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboLayoutBinding.descriptorCount = 1;
    ssboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    return ssboLayoutBinding;
}