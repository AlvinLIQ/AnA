#include "Headers/Mesh.hpp"
#include "Headers/ResourceManager.hpp"
#include "Headers/Texture.hpp"

using namespace AnA;

Meshes::Meshes(Device* mDevice) : aDevice{mDevice}
{
    //auto& properties = aDevice->GetPhysicalDeviceProperties();
    batchSize = MaxBatchSize;
    vertexBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    indexBuffers.resize(vertexBuffers.size());

    meshletsBuffers.resize(vertexBuffers.size());

    drawIndexedIndirectBuffer = Buffer(aDevice, sizeof(VkDrawIndexedIndirectCommand), 
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    drawIndexedIndirectBuffer.Map(0, drawIndexedIndirectBuffer.GetSize());
    auto drawIndexedIndirectCommand = (VkDrawIndexedIndirectCommand*)drawIndexedIndirectBuffer.GetMappedData();
    drawIndexedIndirectCommand->firstIndex = 0;
    drawIndexedIndirectCommand->firstInstance = 0;
    drawIndexedIndirectCommand->instanceCount = 1;
    drawIndexedIndirectCommand->vertexOffset = 0;

    drawMeshIndirectBuffer = Buffer(aDevice, sizeof(VkDrawMeshTasksIndirectCommandEXT), 
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    drawMeshIndirectBuffer.Map(0, drawMeshIndirectBuffer.GetSize());
    auto drawMeshIndirectCommand = (VkDrawMeshTasksIndirectCommandEXT*)drawMeshIndirectBuffer.GetMappedData();
    drawMeshIndirectCommand->groupCountX = 1;
    drawMeshIndirectCommand->groupCountY = 1;
    drawMeshIndirectCommand->groupCountZ = 1;

    countBuffer = Buffer(aDevice, 4, 
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    countBuffer.Map(0, 4);
    *(uint32_t*)countBuffer.GetMappedData() = 1;
}

Meshes::~Meshes()
{
    delete vertexDescriptor;
    delete meshDescriptor;
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

void Meshes::Append(std::vector<Model::Vertex>& vertices, std::vector<uint32_t>& indices, Transform transform)
{
    std::vector<VkDescriptorImageInfo> imageInfos{};

    Mesh mesh{};
    mesh.transform = transform;
    mesh.vertexOffset = vertexCount;
    mesh.indexOffset = indexCount;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vertexCount += mesh.vertices.size();
    indexCount += mesh.indices.size();
    auto& textureMap = Resource::ResourceManager::GetCurrent()->TextureMap;
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
    vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentBufferIndex].GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    ((VkDrawIndexedIndirectCommand*)drawIndexedIndirectBuffer.GetMappedData())->indexCount = indexBuffers[currentBufferIndex].GetSize() / sizeof(Model::Index);
}

void Meshes::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDrawIndexed(commandBuffer, indexBuffers[currentBufferIndex].GetSize() / sizeof(Model::Index), 1, 0, 0, 0);
}

void Meshes::Draw(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout, size_t offset, size_t size)
{
    size_t i, j;
    sets[DEFAULT_VERTEX_LAYOUT] = vertexDescriptor->GetSets()[currentBufferIndex];
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
    vkCmdDrawIndexedIndirectCount(commandBuffer, drawIndexedIndirectBuffer.GetBuffer(), 0, countBuffer.GetBuffer(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
}

void Meshes::DrawIndirect(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout)
{    
    sets[DEFAULT_VERTEX_LAYOUT] = vertexDescriptor->GetSets()[currentBufferIndex];
    sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors.front()->GetSets()[currentBufferIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
    sets.data(), 0, nullptr);

    vkCmdDrawIndexedIndirectCount(commandBuffer, drawIndexedIndirectBuffer.GetBuffer(), 0, countBuffer.GetBuffer(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
}

void Meshes::DrawMesh(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    aDevice->vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Meshes::DrawMesh(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout)
{
    sets[DEFAULT_VERTEX_LAYOUT] = vertexDescriptor->GetSets()[currentBufferIndex];
    sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors.front()->GetSets()[0];
    sets[DEFAULT_MESHLET_LAYOUT] = meshDescriptor->GetSets()[currentBufferIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
    sets.data(), 0, nullptr);

    aDevice->vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Meshes::DrawMeshIndirect(VkCommandBuffer commandBuffer, std::vector<VkDescriptorSet>& sets, VkPipelineLayout pipelineLayout)
{
    sets[DEFAULT_VERTEX_LAYOUT] = vertexDescriptor->GetSets()[currentBufferIndex];
    sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors.front()->GetSets()[0];
    sets[DEFAULT_MESHLET_LAYOUT] = meshDescriptor->GetSets()[currentBufferIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout, 0, static_cast<uint32_t>(sets.size()),
    sets.data(), 0, nullptr);

    aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawMeshIndirectBuffer.GetBuffer(), 0, 
        countBuffer.GetBuffer(), 
        0, 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
}

void Meshes::CommitBufferUpdate(Buffer* newVertBuffer, Buffer* newIndexBuffer)
{
    auto vertices = ((Model::Vertex*)newVertBuffer->GetMappedData());
    auto indices = ((Model::Index*)newIndexBuffer->GetMappedData());

    Range updateRange = {0, (uint32_t)meshes.size()};
    commitVertexBufferUpdate(vertices, indices, updateRange);
}

void Meshes::CommitBufferUpdate()
{
    auto vertices = ((Model::Vertex*)vertexBuffers[currentBufferIndex].GetMappedData());
    auto indices = ((Model::Index*)indexBuffers[currentBufferIndex].GetMappedData());
    for (auto& updateRange : updateQueue)
    {
        commitVertexBufferUpdate(vertices, indices, updateRange);
    }
    updateQueue.clear();
}

void Meshes::UpdateAll()
{
    if (!vertexBuffers[currentBufferIndex].GetBuffer())
    {
        if (!EnableUpdate)
            return;
        vertexBuffers[nextIndex] = Buffer(aDevice, vertexCount * sizeof(Model::Vertex), 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //vertexBuffer->CopyToBuffer(Vertices.data(), Vertices.size() * sizeof(Model::Vertex));

        indexBuffers[nextIndex] = Buffer(aDevice, indexCount * sizeof(Model::Index), 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //indexBuffer->CopyToBuffer(Indices.data(), Indices.size() * sizeof(Model::Index));
        vertexBuffers[nextIndex].Map(0, vertexBuffers[nextIndex].GetSize());
        indexBuffers[nextIndex].Map(0, indexBuffers[nextIndex].GetSize());
        UpdateMeshlets();

        createSSBODescriptor();
        updateSSBODescriptor();
        UpdateBuffers({0, meshes.size()});
        commandBufferNeedUpdate = true;
        return;
    }
    Resource::ResourceManager::GetCurrent()->TaskPool.Enqueue([this]()
    {
        vertexBuffers[nextIndex] = Buffer(aDevice, vertexCount * sizeof(Model::Vertex), 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        indexBuffers[nextIndex] = Buffer(aDevice, indexCount * sizeof(Model::Index), 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vertexBuffers[nextIndex].Map(0, vertexBuffers[nextIndex].GetSize());
        indexBuffers[nextIndex].Map(0, indexBuffers[nextIndex].GetSize());

        
        UpdateMeshlets();
        CommitBufferUpdate(&vertexBuffers[nextIndex], &indexBuffers[nextIndex]);

        updateSSBODescriptor();
        commandBufferNeedUpdate = true;
    });
}

void Meshes::UpdateBuffers(Range updateRange)
{
    if (updateQueue.empty())
        updateQueue.push_back(updateRange);
}

void Meshes::UpdateMeshlets()
{
    buildMeshlets();
    uint32_t minMeshletBufferSize = (meshletVertexCount + meshletIndexCount / 3 + 
        3 * static_cast<uint32_t>(meshlets.size())) * sizeof(uint32_t);
    if (!meshletsBuffers[nextIndex].GetBuffer() || 
        meshletsBuffers[nextIndex].GetSize() < minMeshletBufferSize)
    {
        meshletsBuffers[nextIndex] = Buffer(aDevice, minMeshletBufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        meshletsBuffers[nextIndex].Map(0, meshletsBuffers[nextIndex].GetSize());
    }
    uint32_t bufferId = 0;
    uint32_t* buffer = (uint32_t*)meshletsBuffers[nextIndex].GetMappedData();
    uint32_t vertexOffset = 0, primitiveOffset = 0;
    //buffer[bufferId++] = static_cast<uint32_t>(meshlets.size());
    for (size_t i = 0; i < meshlets.size(); i++)
    {
        auto& meshlet = meshlets[i];
        buffer[bufferId++] = vertexOffset + primitiveOffset + (i * 2) + static_cast<uint32_t>(meshlets.size());
        vertexOffset += meshlet.vertexCount;
        primitiveOffset += meshlet.indexCount / 3;
    }
    for (auto& meshlet : meshlets)
    {
        buffer[bufferId++] = meshlet.vertexCount;
        buffer[bufferId++] = meshlet.indexCount / 3;

        for (uint32_t i = 0; i < meshlet.vertexCount; i++)
            buffer[bufferId++] = meshlet.vertices[i];
        for (uint32_t i = 0; i < meshlet.indexCount; i += 3)
        {
            buffer[bufferId++] = (meshlet.indices[i] << 0u) |
                (meshlet.indices[i + 1] << 8u) |
                (meshlet.indices[i + 2] << 16u);
        }
    }
}

void Meshes::UpdateVertexPositions(Mesh& mesh)
{
    auto vertices = ((Model::Vertex*)vertexBuffers[currentBufferIndex].GetMappedData());
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
    auto vertices = ((Model::Vertex*)vertexBuffers[currentBufferIndex].GetMappedData());
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

void Meshes::commitVertexBufferUpdate(Model::Vertex* vertices, Model::Index* indices, Range& updateRange)
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
        Resource::ResourceManager::GetCurrent()->Shaders[0].GetDescriptors()[DEFAULT_VERTEX_LAYOUT]->GetLayout();
    vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT, 
        1,
        descriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT);
    meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT, 
        1,
        descriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_MESH_BIT_EXT);
}

void Meshes::updateSSBODescriptor()
{    
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = vertexBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = vertexBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
        vertexDescriptor->GetSets()[nextIndex]);

    bufferInfo.buffer = meshletsBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = meshletsBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
        meshDescriptor->GetSets()[nextIndex]);
    currentBufferIndex = nextIndex;
    nextIndex = NextFrameIndex(currentBufferIndex);
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

void Meshes::buildMeshlets(
    uint32_t maxVerticesPerMeshlet,  // max number of vertices per meshlet
    uint32_t maxIndicesPerMeshlet)  // max number of indices per meshlet (i.e., triangles)
{
    meshlets.clear();
    meshletVertexCount = 0;
    meshletIndexCount = 0;
    // Iterate over each mesh
    for (const auto& mesh : meshes)
    {
        uint32_t totalIndices = static_cast<uint32_t>(mesh.indices.size());
        uint32_t indexOffset = 0;
        uint32_t indexEnd;
        
        while  (indexOffset < totalIndices)
        {
            std::unordered_map<uint32_t, uint8_t> vertices{};
            Meshlet meshlet{};
            indexEnd = std::min(indexOffset + maxIndicesPerMeshlet, totalIndices);
            for (; indexOffset < indexEnd; indexOffset+= 3)
            {
                if (vertices.try_emplace(mesh.indices[indexOffset], static_cast<uint8_t>(vertices.size())).second)
                {
                    if (static_cast<uint32_t>(vertices.size()) >= maxVerticesPerMeshlet)
                        break;
                    meshlet.vertices[meshlet.vertexCount++] = mesh.indices[indexOffset];
                }
                if (vertices.try_emplace(mesh.indices[indexOffset + 1], static_cast<uint8_t>(vertices.size())).second)
                {
                    if (static_cast<uint32_t>(vertices.size()) >= maxVerticesPerMeshlet)
                        break;
                    meshlet.vertices[meshlet.vertexCount++] = mesh.indices[indexOffset + 1];
                }
                if (vertices.try_emplace(mesh.indices[indexOffset + 2], static_cast<uint8_t>(vertices.size())).second)
                {
                    if (static_cast<uint32_t>(vertices.size()) > maxVerticesPerMeshlet)
                        break;
                    meshlet.vertices[meshlet.vertexCount++] = mesh.indices[indexOffset + 2];
                }
                meshlet.indices[meshlet.indexCount++] = vertices[mesh.indices[indexOffset]];
                meshlet.indices[meshlet.indexCount++] = vertices[mesh.indices[indexOffset + 1]];
                meshlet.indices[meshlet.indexCount++] = vertices[mesh.indices[indexOffset + 2]];
            }
            if (indexOffset < indexEnd)
                indexOffset -= 3;
            meshlets.push_back(meshlet);
            meshletVertexCount += meshlet.vertexCount;
            meshletIndexCount += meshlet.indexCount;
        }
    }
}