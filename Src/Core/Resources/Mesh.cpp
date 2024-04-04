#include "Headers/Mesh.hpp"
#include "Headers/Texture.hpp"

using namespace AnA;

Meshes::Meshes(Device& mDevice) : aDevice {mDevice}
{
}

Meshes::~Meshes()
{
    if (TestTexture != nullptr)
        delete TestTexture;
    delete vertexBuffer;
    delete indexBuffer;
}

void Meshes::Append(const std::vector<MeshInfo>& meshInfos)
{
    for (auto& meshInfo : meshInfos)
    {
        Mesh mesh;
        mesh.transform = meshInfo.transform;
        mesh.vertexOffset = vertexCount;
        mesh.indexOffset = indexCount;
        Model::CreateMeshFromFile(meshInfo.filePath.c_str(), mesh.vertices, mesh.indices, mesh.vertexOffset);
        vertexCount += mesh.vertices.size();
        indexCount += mesh.indices.size();
        meshes.push_back(mesh);
    }
    vertexBuffer = new Buffer(aDevice, vertexCount * sizeof(Model::Vertex), 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //vertexBuffer->CopyToBuffer(Vertices.data(), Vertices.size() * sizeof(Model::Vertex));

    indexBuffer = new Buffer(aDevice, indexCount * sizeof(Model::Index), 
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //indexBuffer->CopyToBuffer(Indices.data(), Indices.size() * sizeof(Model::Index));
    vertexBuffer->Map(0, vertexBuffer->GetSize());
    indexBuffer->Map(0, indexBuffer->GetSize());
    UpdateBuffers({0, meshes.size()});
}

void Meshes::Bind(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer->GetBuffer(), &offset);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Meshes::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Meshes::CommitBufferUpdate()
{
    auto vertices = ((Model::Vertex*)vertexBuffer->GetMappedData());
    auto indices = ((Model::Index*)indexBuffer->GetMappedData());

    for (auto& updateRange : updateQueue)
    {
        for (uint32_t i = 0; i < updateRange.y; i++)
        {
            auto& mesh = meshes[i + updateRange.x];
            glm::mat3 model = mesh.transform.mat3();
            for (size_t j = 0; j < mesh.vertices.size(); j++)
            {
                auto& vertex = vertices[mesh.vertexOffset + j];
                auto& meshVertex = mesh.vertices[j];
                vertex.position = model * meshVertex.position + mesh.transform.translation;
                vertex.color = meshVertex.color;
                vertex.normal = meshVertex.normal;
                vertex.uv = meshVertex.uv;
            }
            memcpy(&indices[mesh.indexOffset], mesh.indices.data(), mesh.indices.size() * sizeof(Model::Index));
        }
    }
    updateQueue.clear();
}

void Meshes::UpdateBuffers(Range updateRange)
{
    if (updateQueue.empty())
        updateQueue.push_back(updateRange);
}

void Meshes::UpdateVertexPositions(Mesh& mesh)
{
    auto vertices = ((Model::Vertex*)vertexBuffer->GetMappedData());
    glm::mat3 model = mesh.transform.mat3();
    for (size_t i = 0; i < mesh.vertices.size(); i++)
    {
        auto& vertex = vertices[mesh.vertexOffset + i];
        auto& meshVertex = mesh.vertices[i];
        vertex.position = model * meshVertex.position + mesh.transform.translation;
    }
}

void Meshes::UpdateVertexPositions(Range updateRange)
{
    auto vertices = ((Model::Vertex*)vertexBuffer->GetMappedData());
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& mesh = meshes[i];
        glm::mat3 model = mesh.transform.mat3();
        for (size_t i = 0; i < mesh.vertices.size(); i++)
        {
            auto& vertex = vertices[mesh.vertexOffset + i];
            auto& meshVertex = mesh.vertices[i];
            vertex.position = model * meshVertex.position + mesh.transform.translation;
        }
    }
}