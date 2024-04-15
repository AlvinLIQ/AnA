#include "Headers/Mesh.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;

Meshes::Meshes(Device& mDevice) : aDevice {mDevice}
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(mDevice.GetPhysicalDevice(), &properties);
    batchSize = MaxBatchSize;
}

Meshes::~Meshes()
{
    for (auto& descriptor : samplersDescriptors)
        delete descriptor;
    delete vertexBuffer;
    delete indexBuffer;
}

void Meshes::Append(const std::vector<MeshInfo>& meshInfos)
{
    std::vector<VkDescriptorImageInfo> imageInfos{};
    for (auto& meshInfo : meshInfos)
    {
        Mesh mesh;
        mesh.transform = meshInfo.transform;
        mesh.vertexOffset = vertexCount;
        mesh.indexOffset = indexCount;
        Model::CreateMeshFromFile(meshInfo.filePath.c_str(), mesh.vertices, mesh.indices, mesh.vertexOffset);
        vertexCount += mesh.vertices.size();
        indexCount += mesh.indices.size();
        auto& textureMap = Resource::ResourceManager::GetCurrent()->TextureMap;
        mesh.textureId = textureMap.find(meshInfo.tetureId) == textureMap.end() ? DEFAULT_TEXTURE_ID : meshInfo.tetureId;
        auto& texture = textureMap.at(mesh.textureId);
        if (textureIdMap.find(mesh.textureId) == textureIdMap.end())
        {
            textureIdMap.insert(std::pair<uint32_t, uint32_t>(mesh.textureId, static_cast<uint32_t>(textureIdMap.size())));
            imageInfos.push_back(texture.GetImageInfo());
            if (imageInfos.size() == batchSize)
            {
                appendSamplersDescriptor(imageInfos);
                imageInfos.clear();
            }
        }
        meshes.push_back(mesh);
    }
    if (imageInfos.size())
    {
        appendSamplersDescriptor(imageInfos);
    }
    UpdateAll();
}

void Meshes::RemoveAt(uint32_t meshIndex)
{
    meshes.erase(meshes.begin() + meshIndex);
    UpdateAll();
}

void Meshes::RemoveAt(Range removeRange)
{
    for (uint32_t i = 0; i < removeRange.y; i++)
        meshes.erase(meshes.begin() + i + removeRange.x);
    UpdateAll();
}

void Meshes::RemoveAt(std::vector<uint32_t> meshIndices)
{
    for (auto& meshIndex : meshIndices)
        meshes.erase(meshes.begin() + meshIndex);
    UpdateAll();
}

void Meshes::Bind(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer->GetBuffer(), &offset);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Meshes::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDrawIndexed(commandBuffer, indexBuffer->GetSize() / sizeof(Model::Index), 1, 0, 0, 0);
}

void Meshes::Draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout, size_t offset, size_t size)
{
    size_t i, j;
    for (i = batchSize, j = 0; i < size; i += batchSize, j++)
    {
        sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors[j]->GetSets()[0];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);

        auto indexOffset = meshes[i - batchSize].indexOffset;
        vkCmdDrawIndexed(commandBuffer, meshes[i].indexOffset - indexOffset, 
            1, indexOffset, 0, 0);
    }
    if (i > size)
    {
        sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors[j]->GetSets()[0];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);

        auto indexOffset = meshes[i - batchSize].indexOffset;
        auto& backMesh = meshes[size - 1];
        vkCmdDrawIndexed(commandBuffer, backMesh.indexOffset - indexOffset + backMesh.indices.size(), 
            1, indexOffset, 0, 0);
    }
}

void Meshes::CommitBufferUpdate(Buffer* newVertBuffer, Buffer* newIndexBuffer)
{
    auto vertices = ((Model::Vertex*)newVertBuffer->GetMappedData());
    auto indices = ((Model::Index*)newIndexBuffer->GetMappedData());

    Range updateRange = {0, (uint32_t)meshes.size()};
    commitBufferUpdate(vertices, indices, updateRange);
}

void Meshes::CommitBufferUpdate()
{
    auto vertices = ((Model::Vertex*)vertexBuffer->GetMappedData());
    auto indices = ((Model::Index*)indexBuffer->GetMappedData());
    for (auto& updateRange : updateQueue)
    {
        commitBufferUpdate(vertices, indices, updateRange);
    }
    updateQueue.clear();
}

void Meshes::UpdateAll()
{
    if (vertexBuffer == nullptr)
    {
        auto newVertexBuffer = new Buffer(aDevice, vertexCount * sizeof(Model::Vertex), 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //vertexBuffer->CopyToBuffer(Vertices.data(), Vertices.size() * sizeof(Model::Vertex));

        auto newIndexBuffer = new Buffer(aDevice, indexCount * sizeof(Model::Index), 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //indexBuffer->CopyToBuffer(Indices.data(), Indices.size() * sizeof(Model::Index));
        newVertexBuffer->Map(0, newVertexBuffer->GetSize());
        newIndexBuffer->Map(0, newIndexBuffer->GetSize());
        UpdateBuffers({0, meshes.size()});
        vertexBuffer = newVertexBuffer;
        indexBuffer = newIndexBuffer;
        commandBufferNeedUpdate = true;
        return;
    }
    Resource::ResourceManager::GetCurrent()->TaskPool.Enqueue([this]()
    {
        auto newVertexBuffer = new Buffer(aDevice, vertexCount * sizeof(Model::Vertex), 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //vertexBuffer->CopyToBuffer(Vertices.data(), Vertices.size() * sizeof(Model::Vertex));

        auto newIndexBuffer = new Buffer(aDevice, indexCount * sizeof(Model::Index), 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //indexBuffer->CopyToBuffer(Indices.data(), Indices.size() * sizeof(Model::Index));
        newVertexBuffer->Map(0, newVertexBuffer->GetSize());
        newIndexBuffer->Map(0, newIndexBuffer->GetSize());
        CommitBufferUpdate(newVertexBuffer, newIndexBuffer);
        vertexBuffer->ReplaceRequest(newVertexBuffer);
        indexBuffer->ReplaceRequest(newIndexBuffer);
        commandBufferNeedUpdate = true;
    });
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

void Meshes::commitBufferUpdate(Model::Vertex* vertices, Model::Index* indices, Range& updateRange)
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
            vertex.normal = meshVertex.normal;
            vertex.uv = meshVertex.uv;
            vertex.textureId = textureIdMap.at(mesh.textureId);
        }
        memcpy(&indices[mesh.indexOffset], mesh.indices.data(), mesh.indices.size() * sizeof(Model::Index));
    }
}

void Meshes::appendSamplersDescriptor(std::vector<VkDescriptorImageInfo>& imageInfos)
{
    auto& descriptorSetLayout = Resource::ResourceManager::GetCurrent()->Shaders[0]->GetDescriptors()[DEFAULT_SAMPLER_LAYOUT]->GetLayout();
    auto descriptor = new Descriptor(aDevice, 1, 
        batchSize,
        descriptorSetLayout,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptor->UpdateDescriptorSets(imageInfos, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    samplersDescriptors.push_back(descriptor);
}