#include "Headers/Mesh.hpp"
#include "Headers/ResourceManager.hpp"
#include "Headers/Texture.hpp"
#include "../../3rdParty/meshoptimizer/src/meshoptimizer.h"

using namespace AnA;

void FrustumPlanes::ExtractFrustumPlanes(const glm::mat4& m, FrustumPlanes& fp)
{
    //Left
    fp.planes[0] = glm::vec4(
        m[0][3] + m[0][0],
        m[1][3] + m[1][0],
        m[2][3] + m[2][0],
        m[3][3] + m[3][0]);

    // Right
    fp.planes[1] = glm::vec4(
        m[0][3] - m[0][0],
        m[1][3] - m[1][0],
        m[2][3] - m[2][0],
        m[3][3] - m[3][0]);

    // Bottom
    fp.planes[2] = glm::vec4(
        m[0][3] + m[0][1],
        m[1][3] + m[1][1],
        m[2][3] + m[2][1],
        m[3][3] + m[3][1]);

    // Top
    fp.planes[3] = glm::vec4(
        m[0][3] - m[0][1],
        m[1][3] - m[1][1],
        m[2][3] - m[2][1],
        m[3][3] - m[3][1]);

    // Near
    fp.planes[4] = glm::vec4(
        m[0][3] + m[0][2],
        m[1][3] + m[1][2],
        m[2][3] + m[2][2],
        m[3][3] + m[3][2]);

    // Far
    fp.planes[5] = glm::vec4(
        m[0][3] - m[0][2],
        m[1][3] - m[1][2],
        m[2][3] - m[2][2],
        m[3][3] - m[3][2]);

    // Normalize the planes
    for (auto& p : fp.planes)
    {
        float len = glm::length(glm::vec3(p));
        p /= len;
    }
}

Meshes::Meshes(Device* mDevice) : aDevice{mDevice}
{
    //auto& properties = aDevice->GetPhysicalDeviceProperties();
    batchSize = MaxBatchSize;
    numOfGroup = aDevice->GetMeshShaderProperties().maxPreferredTaskWorkGroupInvocations;
    vertexBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    indexBuffers.resize(vertexBuffers.size());

    meshletBuffers.resize(vertexBuffers.size());
    meshletCullingBuffers.resize(meshletBuffers.size());

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
    Append(meshInfos.data(), meshInfos.size());
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
        Model::CreateMeshFromFile(meshInfo.filePath, vertices, indices, mesh.vertexOffset);
        vertexCount = static_cast<uint32_t>(vertices.size());
        indexCount = static_cast<uint32_t>(indices.size());
        mesh.vertexCount = vertexCount - mesh.vertexOffset;
        mesh.indexCount = indexCount - mesh.indexOffset;
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

void Meshes::Append(std::vector<Model::Vertex>& meshVertices, std::vector<uint32_t>& meshIndices, Transform transform)
{
    std::vector<VkDescriptorImageInfo> imageInfos{};

    Mesh mesh{};
    mesh.transform = transform;
    mesh.vertexOffset = vertexCount;
    mesh.indexOffset = indexCount;
    vertices.insert(meshVertices.end(), meshVertices.begin(), meshVertices.end());
    for (auto& index : meshIndices)
        index += vertexCount;
    indices.insert(meshIndices.end(), meshIndices.begin(), meshIndices.end());
    vertexCount = static_cast<uint32_t>(vertices.size());
    indexCount = static_cast<uint32_t>(indices.size());
    mesh.vertexCount = vertexCount - mesh.vertexOffset;
    mesh.indexCount = indexCount - mesh.indexOffset;

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

void Meshes::DrawMesh(VkCommandBuffer commandBuffer)
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

void Meshes::DrawMeshIndirect(VkCommandBuffer commandBuffer)
{
    aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawMeshIndirectBuffer.GetBuffer(), 0, 
        countBuffer.GetBuffer(), 
        0, 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
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
    applyVertexBufferUpdate(vertices, indices, updateRange);
}

void Meshes::CommitBufferUpdate()
{
    auto vertices = ((Model::Vertex*)vertexBuffers[currentBufferIndex].GetMappedData());
    auto indices = ((Model::Index*)indexBuffers[currentBufferIndex].GetMappedData());
    for (auto& updateRange : updateQueue)
    {
        applyVertexBufferUpdate(vertices, indices, updateRange);
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
        CommitBufferUpdate(&vertexBuffers[nextIndex], &indexBuffers[nextIndex]);

        createSSBODescriptor();
        updateSSBODescriptor();
        //UpdateBuffers({0, meshes.size()});
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
    //buildMeshlets();
    buildMeshletsWithOptimizer();
    uint32_t minMeshletBufferSize = (meshletVertexCount + meshletIndexCount / 3 + 1 +
        3 * meshlets.size()) * sizeof(uint32_t);
    if (!meshletBuffers[nextIndex].GetBuffer() || 
        meshletBuffers[nextIndex].GetSize() < minMeshletBufferSize)
    {
        meshletBuffers[nextIndex] = Buffer(aDevice, minMeshletBufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        meshletBuffers[nextIndex].Map(0, meshletBuffers[nextIndex].GetSize());

        meshletCullingBuffers[nextIndex] = Buffer(aDevice, meshlets.size() * sizeof(glm::vec4),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        meshletCullingBuffers[nextIndex].Map(0, meshletCullingBuffers[nextIndex].GetSize());
    }
    uint32_t bufferId = 0;
    uint32_t* buffer = (uint32_t*)meshletBuffers[nextIndex].GetMappedData();
    glm::vec4* cullingBuffer = (glm::vec4*)meshletCullingBuffers[nextIndex].GetMappedData();
    uint32_t vertexOffset = 0, primitiveOffset = 0;
    if (meshlets.empty())
        buffer[0] = 0;
    for (size_t i = 0; i < meshlets.size(); i++)
    {
        auto& meshlet = meshlets[i];
        buffer[bufferId++] = vertexOffset + primitiveOffset + (i * 2) + static_cast<uint32_t>(meshlets.size());
        vertexOffset += meshlet.vertexCount;
        primitiveOffset += (meshlet.indexCount >> 2);
        if (meshlet.indexCount & 3)
            primitiveOffset++;

        cullingBuffer[i] = glm::vec4(meshlet.center, meshlet.radius);
    }
    for (auto& meshlet : meshlets)
    {
        buffer[bufferId++] = meshlet.vertexCount;
        buffer[bufferId++] = meshlet.indexCount / 3;

        for (uint32_t i = 0; i < meshlet.vertexCount; i++)
            buffer[bufferId++] = meshlet.vertices[i];
        auto indexBuffer = (uint8_t*)(&buffer[bufferId]);
        for (uint32_t i = 0; i < meshlet.indexCount; i++)
        {
            indexBuffer[i] = meshlet.indices[i];
        }
        bufferId += (meshlet.indexCount >> 2);
        if (meshlet.indexCount & 3)
            bufferId++;
        /*
        for (uint32_t i = 0; i < meshlet.indexCount; i += 3)
        {
            buffer[bufferId++] = (meshlet.indices[i] << 0u) |
                (meshlet.indices[i + 1] << 8u) |
                (meshlet.indices[i + 2] << 16u);
        }*/
    }

    auto drawMeshTaskCommand = (glm::uvec3*)drawMeshIndirectBuffer.GetMappedData();
    uint32_t numofGroup = (static_cast<uint32_t>(meshlets.size()) + numOfGroup - 1) / numOfGroup;
    glm::uvec3 groupSize;
    groupSize.x = numofGroup;
    groupSize.y = 1;
    groupSize.z = 1;
    *drawMeshTaskCommand = groupSize;
}

void Meshes::UpdateVertexPositions(Mesh& mesh)
{
    auto vertexBufferData = ((Model::Vertex*)vertexBuffers[currentBufferIndex].GetMappedData());
    glm::mat3 model = mesh.transform.mat3();
    for (size_t i = 0; i < mesh.vertexCount; i++)
    {
        auto& vertex = vertexBufferData[mesh.vertexOffset + i];
        auto& meshVertex = vertices[mesh.vertexOffset + i];
        vertex.position = model * meshVertex.position + mesh.transform.translation;
    }
}

void Meshes::UpdateVertexPositions(Range updateRange)
{
    auto vertexBufferData = ((Model::Vertex*)vertexBuffers[currentBufferIndex].GetMappedData());
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& mesh = meshes[updateRange.x + i];
        glm::mat3 model = mesh.transform.mat3();
        for (size_t j = 0; i < mesh.vertexCount; j++)
        {
            auto& vertex = vertexBufferData[mesh.vertexOffset + j];
            auto& meshVertex = vertices[mesh.vertexOffset + j];
            vertex.position = model * meshVertex.position + mesh.transform.translation;
        }
    }
}

void Meshes::applyVertexBufferUpdate(Model::Vertex* vertexBufferData, Model::Index* indexBufferData, Range& updateRange)
{
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& mesh = meshes[i + updateRange.x];
        glm::mat3 model = mesh.transform.mat3();
        for (size_t j = 0; j < mesh.vertexCount; j++)
        {
            auto& vertex = vertexBufferData[mesh.vertexOffset + j];
            auto& meshVertex = vertices[mesh.vertexOffset + j];
            vertex.position = model * meshVertex.position + mesh.transform.translation;
            vertex.normal = meshVertex.normal;
            vertex.uv = meshVertex.uv;
            vertex.textureId = textureIdMap.at(mesh.textureId);
        }
        memcpy(&indexBufferData[mesh.indexOffset], &indices[mesh.indexOffset], mesh.indexCount * sizeof(Model::Index));
    }
}

void Meshes::createSSBODescriptor()
{
    auto& shaders = Resource::ResourceManager::GetCurrent()->Shaders;
    auto& vertexDescriptorSetLayout = 
        shaders.front().GetDescriptors()[DEFAULT_VERTEX_LAYOUT]->GetLayout();
    auto& meshDescriptorSetLayout = shaders.back().GetDescriptors()[DEFAULT_MESHLET_LAYOUT]->GetLayout();
    vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT, 
        1,
        vertexDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT, 
        2,
        meshDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

void Meshes::updateSSBODescriptor()
{    
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = vertexBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = vertexBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
        vertexDescriptor->GetSets()[nextIndex]);

    bufferInfo.buffer = meshletBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = meshletBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 

    meshDescriptor->GetSets()[nextIndex]);
    bufferInfo.buffer = meshletCullingBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = meshletCullingBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
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
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );
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
        uint32_t totalIndices = mesh.indexCount;
        uint32_t indexOffset = 0;
        uint32_t indexEnd;
        
        while  (indexOffset < totalIndices)
        {
            std::unordered_map<uint32_t, uint8_t> vertexMap{};
            Meshlet meshlet{};
            indexEnd = std::min(indexOffset + maxIndicesPerMeshlet, totalIndices);
            for (; indexOffset < indexEnd; indexOffset+= 3)
            {
                uint32_t iid = indexOffset + mesh.indexOffset;
                if (vertexMap.try_emplace(indices[iid], static_cast<uint8_t>(vertexMap.size())).second)
                {
                    if (static_cast<uint32_t>(vertices.size()) >= maxVerticesPerMeshlet)
                        break;
                    meshlet.vertices[meshlet.vertexCount++] = indices[iid];
                }
                if (vertexMap.try_emplace(indices[iid + 1], static_cast<uint8_t>(vertexMap.size())).second)
                {
                    if (static_cast<uint32_t>(vertices.size()) >= maxVerticesPerMeshlet)
                        break;
                    meshlet.vertices[meshlet.vertexCount++] = indices[iid + 1];
                }
                if (vertexMap.try_emplace(indices[iid + 2], static_cast<uint8_t>(vertexMap.size())).second)
                {
                    if (static_cast<uint32_t>(vertices.size()) > maxVerticesPerMeshlet)
                        break;
                    meshlet.vertices[meshlet.vertexCount++] = indices[iid + 2];
                }
                meshlet.indices[meshlet.indexCount++] = vertexMap[indices[iid + 0]];
                meshlet.indices[meshlet.indexCount++] = vertexMap[indices[iid + 1]];
                meshlet.indices[meshlet.indexCount++] = vertexMap[indices[iid + 2]];
            }
            if (indexOffset < indexEnd)
                indexOffset -= 3;
            meshlets.push_back(meshlet);
            meshletVertexCount += meshlet.vertexCount;
            meshletIndexCount += meshlet.indexCount;
        }
    }
}

void Meshes::buildMeshletsWithOptimizer()
{
    constexpr size_t MaxVerts = 128;
    constexpr size_t MaxTris = 256;

    meshlets.clear();
    meshletVertexCount = 0;
    meshletIndexCount = 0;

    for (const auto& mesh : meshes)
    {
        std::vector<Model::Index> meshIndices(mesh.indexCount);
        for (uint32_t i = 0; i < mesh.indexCount; i++)
            meshIndices[i] = indices[mesh.indexOffset + i] - mesh.vertexOffset;
        // Estimate output sizes
        size_t maxMeshlets = meshopt_buildMeshletsBound(mesh.indexCount, MaxVerts, MaxTris);

        std::vector<meshopt_Meshlet> meshopt_meshlets(maxMeshlets);
        std::vector<uint32_t> uniqueVertexIndices(maxMeshlets * MaxVerts);
        std::vector<uint8_t> primitiveIndices(maxMeshlets * MaxTris * 3);

        size_t actualMeshletCount = meshopt_buildMeshlets(
            meshopt_meshlets.data(),
            uniqueVertexIndices.data(),
            primitiveIndices.data(),
            meshIndices.data(),
            meshIndices.size(),
            &vertices[mesh.vertexOffset].position.x, // Optional vertex data pointer, can be nullptr
            mesh.vertexCount,
            sizeof(Model::Vertex),
            MaxVerts,
            MaxTris,
            0 // flags
        );

        meshopt_meshlets.resize(actualMeshletCount);
        auto& last = meshopt_meshlets.back();
        uniqueVertexIndices.resize(last.vertex_offset + last.vertex_count);
        primitiveIndices.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));

        for (auto& meshletInfo : meshopt_meshlets)
        {
            glm::vec3 minBounding{std::numeric_limits<float>::max()};
            glm::vec3 maxBounding{-std::numeric_limits<float>::max()};
            Meshlet meshlet{};
            meshlet.indexCount = static_cast<uint32_t>(meshletInfo.triangle_count) * 3;
            meshlet.vertexCount = static_cast<uint32_t>(meshletInfo.vertex_count);
            meshletIndexCount += meshlet.indexCount;
            meshletVertexCount += meshlet.vertexCount;
            for (uint32_t i = 0; i < meshlet.indexCount; i++)
            {
                meshlet.indices[i] = primitiveIndices[i + meshletInfo.triangle_offset];
            }
            for (uint32_t i = 0; i < meshlet.vertexCount; i++)
            {
                meshlet.vertices[i] = uniqueVertexIndices[i + meshletInfo.vertex_offset] + mesh.vertexOffset;
                minBounding = glm::min(minBounding, vertices[meshlet.vertices[i]].position);
                maxBounding = glm::max(maxBounding, vertices[meshlet.vertices[i]].position);
            }
            meshlet.center = (minBounding + maxBounding) * 0.5f;
            auto model = mesh.transform.mat3();
            meshlet.center = model * meshlet.center + mesh.transform.translation;
            for (uint32_t i = 0; i < meshlet.vertexCount; i++)
            {
                float distance = glm::distance(meshlet.center, 
                    model * vertices[uniqueVertexIndices[i + meshletInfo.vertex_offset] + mesh.vertexOffset].position + mesh.transform.translation);
                if (distance > meshlet.radius)
                    meshlet.radius = distance;

            }
            meshlets.push_back(meshlet);
        }
    }
}