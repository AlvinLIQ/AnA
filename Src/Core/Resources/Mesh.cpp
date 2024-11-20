#include "Headers/Mesh.hpp"
#include "Headers/ResourceManager.hpp"
#include "Headers/Texture.hpp"

using namespace AnA;

Meshes::Meshes(Device* mDevice) : aDevice{mDevice}
{
    auto& properties = aDevice->GetPhysicalDeviceProperties();
    batchSize = MaxBatchSize;
    indirectBuffer = Buffer(aDevice, sizeof(VkDrawIndexedIndirectCommand), 
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    indirectBuffer.Map(0, indirectBuffer.GetSize());
    auto indirectCommand = (VkDrawIndexedIndirectCommand*)indirectBuffer.GetMappedData();
    indirectCommand->firstIndex = 0;
    indirectCommand->firstInstance = 0;
    indirectCommand->instanceCount = 1;
    indirectCommand->vertexOffset = 0;
    countBuffer = Buffer(aDevice, 4, 
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    countBuffer.Map(0, 4);
    *(uint32_t*)countBuffer.GetMappedData() = 1;
}

Meshes::~Meshes()
{
    delete ssboDescriptor;
    for (auto& descriptor : samplersDescriptors)
        delete descriptor;
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
        Model::CreateMeshFromFile(meshInfo.filePath, mesh.vertices, mesh.indices, mesh.vertexOffset);
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

void Meshes::Append(const MeshInfo* meshInfos, size_t count)
{
    std::vector<VkDescriptorImageInfo> imageInfos{};
    for (size_t i = 0; i < count; i++)
    {
        auto& meshInfo = meshInfos[i];
        Mesh mesh;
        mesh.transform = meshInfo.transform;
        mesh.vertexOffset = vertexCount;
        mesh.indexOffset = indexCount;
        Model::CreateMeshFromFile(meshInfo.filePath, mesh.vertices, mesh.indices, mesh.vertexOffset);
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
    //see you later buddy
    //vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.GetBuffer(), &offset);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    ((VkDrawIndexedIndirectCommand*)indirectBuffer.GetMappedData())->indexCount = indexBuffer.GetSize() / sizeof(Model::Index);
}

void Meshes::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDrawIndexed(commandBuffer, indexBuffer.GetSize() / sizeof(Model::Index), 1, 0, 0, 0);
}

void Meshes::Draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout, size_t offset, size_t size)
{
    size_t i, j;
    sets[DEFAULT_SSBO_LAYOUT] = ssboDescriptor->GetSets()[0];
    for (i = batchSize, j = 0; i < size; i += batchSize, j++)
    {
        sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors[j]->GetSets()[j];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);

        auto indexOffset = meshes[i - batchSize].indexOffset;
        vkCmdDrawIndexed(commandBuffer, meshes[i].indexOffset - indexOffset, 
            1, indexOffset, 0, 0);
    }
    if (i > size)
    {
        sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors[j]->GetSets()[j];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);

        auto indexOffset = meshes[i - batchSize].indexOffset;
        auto& backMesh = meshes[size - 1];
        vkCmdDrawIndexed(commandBuffer, backMesh.indexOffset - indexOffset + backMesh.indices.size(), 
            1, indexOffset, 0, 0);
    }
}

void Meshes::DrawIndirect(VkCommandBuffer commandBuffer)
{
    vkCmdDrawIndexedIndirectCount(commandBuffer, indirectBuffer.GetBuffer(), 0, countBuffer.GetBuffer(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
}

void Meshes::DrawIndirect(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout)
{    
    sets[DEFAULT_SSBO_LAYOUT] = ssboDescriptor->GetSets()[0];
    sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors.front()->GetSets()[0];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
    sets.data(), 0, nullptr);

    vkCmdDrawIndexedIndirectCount(commandBuffer, indirectBuffer.GetBuffer(), 0, countBuffer.GetBuffer(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
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
    auto vertices = ((Model::Vertex*)vertexBuffer.GetMappedData());
    auto indices = ((Model::Index*)indexBuffer.GetMappedData());
    for (auto& updateRange : updateQueue)
    {
        commitBufferUpdate(vertices, indices, updateRange);
    }
    updateQueue.clear();
}

void Meshes::UpdateAll()
{
    if (!vertexBuffer.GetBuffer())
    {
        auto newVertexBuffer = Buffer(aDevice, vertexCount * sizeof(Model::Vertex), 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //vertexBuffer->CopyToBuffer(Vertices.data(), Vertices.size() * sizeof(Model::Vertex));

        auto newIndexBuffer = Buffer(aDevice, indexCount * sizeof(Model::Index), 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //indexBuffer->CopyToBuffer(Indices.data(), Indices.size() * sizeof(Model::Index));
        newVertexBuffer.Map(0, newVertexBuffer.GetSize());
        newIndexBuffer.Map(0, newIndexBuffer.GetSize());
        UpdateBuffers({0, meshes.size()});
        vertexBuffer = newVertexBuffer;
        indexBuffer = newIndexBuffer;
        createSSBODescriptor();
        updateSSBODescriptor();
        commandBufferNeedUpdate = true;
        return;
    }
    Resource::ResourceManager::GetCurrent()->TaskPool.Enqueue([this]()
    {
        auto newVertexBuffer = new Buffer(aDevice, vertexCount * sizeof(Model::Vertex), 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //vertexBuffer->CopyToBuffer(Vertices.data(), Vertices.size() * sizeof(Model::Vertex));

        auto newIndexBuffer = new Buffer(aDevice, indexCount * sizeof(Model::Index), 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //indexBuffer->CopyToBuffer(Indices.data(), Indices.size() * sizeof(Model::Index));
        newVertexBuffer->Map(0, newVertexBuffer->GetSize());
        newIndexBuffer->Map(0, newIndexBuffer->GetSize());
        CommitBufferUpdate(newVertexBuffer, newIndexBuffer);
        vertexBuffer.ReplaceRequest(newVertexBuffer);
        indexBuffer.ReplaceRequest(newIndexBuffer);
        updateSSBODescriptor();
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
    auto vertices = ((Model::Vertex*)vertexBuffer.GetMappedData());
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
    auto vertices = ((Model::Vertex*)vertexBuffer.GetMappedData());
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& mesh = meshes[updateRange.x + i];
        glm::mat3 model = mesh.transform.mat3();
        for (size_t j = 0; i < mesh.vertices.size(); j++)
        {
            auto& vertex = vertices[mesh.vertexOffset + j];
            auto& meshVertex = mesh.vertices[j];
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

void Meshes::createSSBODescriptor()
{
    auto& descriptorSetLayout = 
        Resource::ResourceManager::GetCurrent()->Shaders[0].GetDescriptors()[DEFAULT_SSBO_LAYOUT]->GetLayout();
    ssboDescriptor = new Descriptor(aDevice, 1, 
        batchSize,
        descriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT);
}

void Meshes::updateSSBODescriptor()
{
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = vertexBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = vertexBuffer.GetSize();
    ssboDescriptor->UpdateDescriptorSets(&bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

void Meshes::appendSamplersDescriptor(std::vector<VkDescriptorImageInfo>& imageInfos)
{
    auto& descriptorSetLayout = 
        Resource::ResourceManager::GetCurrent()->Shaders[0].GetDescriptors()[DEFAULT_SAMPLER_LAYOUT]->GetLayout();
    uint32_t remaining = (uint32_t)textureInfos.size() % batchSize;
    uint32_t offset = (uint32_t)textureInfos.size() - remaining;
    textureInfos.insert(textureInfos.end(), imageInfos.begin(), imageInfos.end());
    if (remaining && samplersDescriptors.size())
    {
        remaining = std::min(remaining + (uint32_t)imageInfos.size(), batchSize);
        samplersDescriptors.back()->UpdateDescriptorSets(&textureInfos[offset], remaining, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        offset += remaining;
    }

    if (offset < textureInfos.size())
    {
        auto descriptor = new Descriptor(aDevice, 1, 
            batchSize,
            descriptorSetLayout,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT);
        descriptor->UpdateDescriptorSets(&textureInfos[offset], textureInfos.size() - offset, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        samplersDescriptors.push_back(descriptor);
    }
}