#include "Headers/Scene.hpp"
#include "Headers/ResourceManager.hpp"
#include "Headers/Texture.hpp"

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

Scene::Scene(Device* mDevice) : aDevice{mDevice}
{
    batchSize = MaxBatchSize;
    numOfGroup = aDevice->GetMeshShaderProperties().maxPreferredTaskWorkGroupInvocations;
}

Scene::~Scene()
{
    if (vertexDescriptor != nullptr)
        delete vertexDescriptor;
    if (meshDescriptor != nullptr)
        delete meshDescriptor;
    for (auto& descriptor : samplersDescriptors)
        delete descriptor;
}

void Scene::Init()
{
    vertexBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    indexBuffers.resize(vertexBuffers.size());

    meshletBuffers.resize(vertexBuffers.size());
    meshletCullingBuffers.resize(meshletBuffers.size());
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vertexBuffers[i] = Buffer(aDevice, (vertexCount + 1000) * sizeof(Model::Vertex),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        vertexBuffers[i].Map();

        indexBuffers[i] = Buffer(aDevice, (indexCount + 1000) * sizeof(Model::Index),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        indexBuffers[i].Map();

        meshletBuffers[i] = Buffer(aDevice, 100 * sizeof(Meshlet),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        meshletBuffers[i].Map();

        meshletCullingBuffers[i] = Buffer(aDevice, 100 * sizeof(BoundingSphere),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        meshletCullingBuffers[i].Map();
    }
    createSamplerDescriptor();
    //createBuffers();
    createIndirectBuffers();
    createSSBODescriptor();
}

void Scene::Append(const std::vector<MeshInfo>& meshInfos)
{
    Append(meshInfos.data(), meshInfos.size());
}

void Scene::Append(const MeshInfo* meshInfos, size_t count)
{
    std::vector<VkDescriptorImageInfo> imageInfos{};
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    for (size_t i = 0; i < count; i++)
    {
        auto& meshInfo = meshInfos[i];
        uint32_t meshId;
        Mesh mesh;
        mesh.transform = meshInfo.transform;
        mesh.vertexOffset = static_cast<uint32_t>(vertexCount);
        mesh.indexOffset = static_cast<uint32_t>(indexCount);
        bool newModel = resourceManager->CreateModel(meshInfo.filePath, meshId);
        mesh.model = resourceManager->ModelMap[meshId];
        mesh.vertexCount = uint32_t(mesh.model->info.vertices.size());
        mesh.indexCount = uint32_t(mesh.model->info.indices.size());
        if (newModel)
        {
            // fix this later
        }
        vertexCount += mesh.vertexCount;
        indexCount += mesh.indexCount;
        meshletIndexCount += mesh.model->meshletIndexCount;
        meshletVertexCount += mesh.model->meshletVertexCount;
        meshletCount += uint32_t(mesh.model->meshlets.size());
        uniqueMeshlets.push_back(&mesh.model->meshlets);

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
    Update();
}

void Scene::Append(std::vector<Model::Vertex>& meshVertices, std::vector<uint32_t>& meshIndices, Transform transform, uint32_t textureId)
{
    std::vector<VkDescriptorImageInfo> imageInfos{};

    Mesh mesh{};
    mesh.transform = transform;
    mesh.vertexOffset = static_cast<uint32_t>(vertexCount);
    mesh.indexOffset = static_cast<uint32_t>(indexCount);
    mesh.vertexCount = static_cast<uint32_t>(meshVertices.size());
    mesh.indexCount = static_cast<uint32_t>(meshIndices.size());
    mesh.textureId = textureId;
    //temporary solution for now
    Model::ModelInfo info{meshVertices, {}, uint32_t(meshIndices.size()), meshIndices};
    mesh.model = std::make_shared<Model>(info);
    vertexCount += mesh.vertexCount;
    indexCount += mesh.indexCount;
    meshletIndexCount += uint32_t(mesh.model->meshletIndexCount);
    meshletVertexCount += uint32_t(mesh.model->meshletVertexCount);
    meshletCount += uint32_t(mesh.model->meshlets.size());
    uniqueMeshlets.push_back(&mesh.model->meshlets);

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
    Update();
}

void Scene::RemoveAt(uint32_t meshIndex)
{
    meshes.erase(meshes.begin() + meshIndex);
    Update();
}

void Scene::RemoveAt(Range removeRange)
{
    for (uint32_t i = 0; i < removeRange.y; i++)
        meshes.erase(meshes.begin() + i + removeRange.x);
    Update();
}

void Scene::RemoveAt(std::vector<uint32_t> meshIndices)
{
    for (auto& meshIndex : meshIndices)
        meshes.erase(meshes.begin() + meshIndex);
    Update();
}

void Scene::Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex)
{
    auto& sets = shader.GetDescriptorSets()[bufferIndex];
    shader.GetPipeline().Bind(commandBuffer);
    if (shader.HasMeshShader())
    {
        sets[DEFAULT_MESHLET_LAYOUT] = meshDescriptor->GetSets()[currentBufferIndex];
    }
    else
    {
        vkCmdSetPrimitiveTopology(commandBuffer, Topology);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentBufferIndex].GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        static_cast<VkDrawIndexedIndirectCommand*>(drawIndexedIndirectBuffer.GetMappedData())->indexCount = static_cast<uint32_t>(indexBuffers[currentBufferIndex].GetSize() / sizeof(Model::Index));
    }
    sets[DEFAULT_VERTEX_LAYOUT] = vertexDescriptor->GetSets()[currentBufferIndex];
    sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors.front()->GetSets()[0];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, static_cast<uint32_t>(sets.size()),
        sets.data(), 0, nullptr);
}

void Scene::Draw(CommandBuffer& commandBuffer)
{
    vkCmdDrawIndexedIndirectCount(commandBuffer, drawIndexedIndirectBuffer.GetBuffer(),
    0, countBuffer.GetBuffer(),
    0, 1, sizeof(VkDrawIndexedIndirectCommand));
}

void Scene::DrawIndirect(CommandBuffer& commandBuffer)
{
    if (aDevice->MeshShaderSupport())
        aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawMeshIndirectBuffer.GetBuffer(), 0,
            countBuffer.GetBuffer(),
            0, 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    else
        vkCmdDrawIndexedIndirectCount(commandBuffer, drawIndexedIndirectBuffer.GetBuffer(),
            0, countBuffer.GetBuffer(),
            0, 1, sizeof(VkDrawIndexedIndirectCommand));
}

void Scene::CommitBufferUpdate(Buffer* newVertBuffer, Buffer* newIndexBuffer)
{
    auto bufferVertices = static_cast<Model::Vertex*>(newVertBuffer->GetMappedData());
    auto bufferIndices = static_cast<Model::Index*>(newIndexBuffer->GetMappedData());

    Range updateRange = {0, static_cast<uint32_t>(meshes.size())};
    applyVertexBufferUpdate(bufferVertices, bufferIndices, updateRange);
}

void Scene::CommitBufferUpdate()
{
    auto bufferVertices = static_cast<Model::Vertex*>(vertexBuffers[currentBufferIndex].GetMappedData());
    auto bufferIndices = static_cast<Model::Index*>(indexBuffers[currentBufferIndex].GetMappedData());
    for (auto& updateRange : updateQueue)
    {
        applyVertexBufferUpdate(bufferVertices, bufferIndices, updateRange);
    }
    updateQueue.clear();
}

void Scene::Update()
{
    if (IsRunning)
    {
        Resource::ResourceManager::GetCurrent()->TaskPool.Enqueue([this]()
        {
            this->updateAll();
        });
    }
    else
    {
        updateAll();
    }
}


void Scene::UpdateBuffers(Range updateRange)
{
    if (updateQueue.empty())
        updateQueue.push_back(updateRange);
}

void Scene::UpdateMeshlets()
{
    if (!aDevice->MeshShaderSupport())
        return;
    uint32_t minMeshletBufferSize = (meshletVertexCount + 1 +
        3 * static_cast<uint32_t>(meshletCount)) * sizeof(uint32_t) + meshletIndexCount + static_cast<uint32_t>(meshletCount);
    if (!meshletBuffers[nextIndex].GetBuffer() ||
        meshletBuffers[nextIndex].GetSize() < minMeshletBufferSize)
    {
        meshletBuffers[nextIndex] = Buffer(aDevice, minMeshletBufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        meshletBuffers[nextIndex].Map();

        meshletCullingBuffers[nextIndex] = Buffer(aDevice, meshletCount * sizeof(BoundingSphere),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        meshletCullingBuffers[nextIndex].Map();
    }
    uint32_t bufferId = 0;
    uint32_t* buffer = static_cast<uint32_t*>(meshletBuffers[nextIndex].GetMappedData());
    BoundingSphere* cullingBuffer = static_cast<BoundingSphere*>(meshletCullingBuffers[nextIndex].GetMappedData());
    uint32_t vertexOffset = 0, primitiveOffset = 0;
    if (!meshletCount)
        buffer[0] = 0;
    glm::vec3 t;
    float len;
    for (size_t j = 0, i = 0; j < meshes.size(); j++)
    {
        auto& mesh = meshes[j];
        auto& meshlets = mesh.model->meshlets;
        auto model = mesh.transform.mat4();
        for (auto& meshlet : meshlets)
        {
            buffer[bufferId++] = vertexOffset + primitiveOffset + static_cast<uint32_t>(i << 1) + static_cast<uint32_t>(meshletCount);
            vertexOffset += meshlet.vertexCount;
            primitiveOffset += (meshlet.indexCount >> 2);
            if (meshlet.indexCount & 3)
                primitiveOffset++;

            t = model * glm::vec4(meshlet.center, 1.0f);
            cullingBuffer[i].center = t;
            cullingBuffer[i].radius = glm::distance(t, 
                glm::vec3(model * glm::vec4(mesh.model->info.vertices[meshlet.vertices[meshlet.farVertexID]].position, 1.0f)));
            t = glm::mat3(model) * meshlet.normal;
            len = glm::length(t);

            cullingBuffer[i].normal = len == 0. ? glm::vec3(1.f, 0.f, 0.f) : t / len;
            cullingBuffer[i].coneApex = model * glm::vec4(meshlet.coneApex, 1.0f);
            cullingBuffer[i].cutoff = meshlet.cutoff;
            ++i;
        }
    }
    for (auto& mesh : meshes)
    {
        auto& meshlets = mesh.model->meshlets;
        for (auto& meshlet : meshlets)
        {
            buffer[bufferId++] = meshlet.vertexCount;
            buffer[bufferId++] = meshlet.indexCount / 3;

            for (uint32_t i = 0; i < meshlet.vertexCount; i++)
                buffer[bufferId++] = meshlet.vertices[i] + mesh.vertexOffset;
            auto indexBuffer = reinterpret_cast<uint8_t*>(&buffer[bufferId]);
            for (uint32_t i = 0; i < meshlet.indexCount; i++)
            {
                indexBuffer[i] = meshlet.indices[i];
            }
            bufferId += (meshlet.indexCount >> 2);
            if (meshlet.indexCount & 3)
                bufferId++;
        }
    }

    auto drawMeshTaskCommand = static_cast<glm::uvec3*>(drawMeshIndirectBuffer.GetMappedData());
    uint32_t numofGroup = (static_cast<uint32_t>(meshletCount) + numOfGroup - 1) / numOfGroup;
    glm::uvec3 groupSize;
    groupSize.x = numofGroup;
    groupSize.y = 1;
    groupSize.z = 1;
    *drawMeshTaskCommand = groupSize;

    //meshletBuffers[nextIndex].Flush();
}

void Scene::UpdateVertexPositions(Mesh& mesh)
{
    auto vertexBufferData = static_cast<Model::Vertex*>(vertexBuffers[currentBufferIndex].GetMappedData());
    glm::mat3 model = mesh.transform.mat3();
    for (size_t i = 0; i < mesh.vertexCount; i++)
    {
        auto& vertex = vertexBufferData[mesh.vertexOffset + i];
        auto& meshVertex = mesh.model->info.vertices[i];
        vertex.position = model * meshVertex.position + mesh.transform.translation;
    }
}

void Scene::UpdateVertexPositions(Range updateRange)
{
    auto vertexBufferData = static_cast<Model::Vertex*>(vertexBuffers[currentBufferIndex].GetMappedData());
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& mesh = meshes[updateRange.x + i];
        glm::mat3 model = mesh.transform.mat3();
        for (size_t j = 0; i < mesh.vertexCount; j++)
        {
            auto& vertex = vertexBufferData[mesh.vertexOffset + j];
            auto& meshVertex = mesh.model->info.vertices[j];
            vertex.position = model * meshVertex.position + mesh.transform.translation;
        }
    }
}

void Scene::applyVertexBufferUpdate(Model::Vertex* vertexBufferData, Model::Index* indexBufferData, Range& updateRange)
{
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& mesh = meshes[i + updateRange.x];
        glm::mat3 model = mesh.transform.mat3();
        for (uint32_t j = 0; j < mesh.vertexCount; j++)
        {
            auto& vertex = vertexBufferData[mesh.vertexOffset + j];
            auto& meshVertex = mesh.model->info.vertices[j];
            vertex.position = model * meshVertex.position + mesh.transform.translation;
            vertex.normal = meshVertex.normal;
            vertex.uv = meshVertex.uv;
            vertex.textureId = textureIdMap.at(mesh.textureId);
        }
        for (uint32_t j = 0; j < mesh.indexCount; j++)
        {
            indexBufferData[mesh.indexOffset + j] = mesh.model->info.indices[j] + mesh.vertexOffset;
        }
    }
    vertexBuffers[nextIndex].Flush();
}

void Scene::createIndirectBuffers()
{
    drawIndexedIndirectBuffer = Buffer(aDevice, sizeof(VkDrawIndexedIndirectCommand),
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    drawIndexedIndirectBuffer.Map();
    auto drawIndexedIndirectCommand =
        static_cast<VkDrawIndexedIndirectCommand*>(drawIndexedIndirectBuffer.GetMappedData());
    drawIndexedIndirectCommand->firstIndex = 0;
    drawIndexedIndirectCommand->firstInstance = 0;
    drawIndexedIndirectCommand->instanceCount = 1;
    drawIndexedIndirectCommand->vertexOffset = 0;

    drawMeshIndirectBuffer = Buffer(aDevice, sizeof(VkDrawMeshTasksIndirectCommandEXT),
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    drawMeshIndirectBuffer.Map();
    auto drawMeshIndirectCommand = static_cast<VkDrawMeshTasksIndirectCommandEXT*>(drawMeshIndirectBuffer.GetMappedData());
    drawMeshIndirectCommand->groupCountX = 1;
    drawMeshIndirectCommand->groupCountY = 1;
    drawMeshIndirectCommand->groupCountZ = 1;

    countBuffer = Buffer(aDevice, 4,
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    countBuffer.Map();
    *static_cast<uint32_t*>(countBuffer.GetMappedData()) = 1;
}

void Scene::createSSBODescriptor()
{
    auto& shaders = Resource::ResourceManager::GetCurrent()->Shaders;
    auto& vertexDescriptorSetLayout =
        shaders.front().GetDescriptors()[DEFAULT_VERTEX_LAYOUT].GetLayout();
    vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
        MaxBatchSize,
        2,
        vertexDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (aDevice->MeshShaderSupport())
    {
        auto& meshDescriptorSetLayout = shaders.back().GetDescriptors()[DEFAULT_MESHLET_LAYOUT].GetLayout();
        meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
            MaxBatchSize * 2,
            2,
            meshDescriptorSetLayout,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
}

void Scene::updateSSBODescriptor()
{
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = vertexBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = vertexBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        vertexDescriptor->GetSets()[nextIndex]);
    if (aDevice->MeshShaderSupport())
    {
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
    }
    currentBufferIndex = nextIndex;
    nextIndex = NextFrameIndex(currentBufferIndex);

    commandBufferNeedUpdate = true;
}

void Scene::appendSamplersDescriptor(std::vector<VkDescriptorImageInfo>& imageInfos)
{
    uint32_t remaining = static_cast<uint32_t>(textureInfos.size()) % batchSize;
    uint32_t offset = static_cast<uint32_t>(textureInfos.size()) - remaining;
    textureInfos.insert(textureInfos.end(), imageInfos.begin(), imageInfos.end());
    if (batchSize - remaining && samplersDescriptors.size())
    {
        remaining = std::min(remaining + static_cast<uint32_t>(imageInfos.size()), batchSize);
        samplersDescriptors.back()->UpdateDescriptorSets(&textureInfos[offset], remaining, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        offset += remaining;
    }

    if (offset < textureInfos.size())
    {
        createSamplerDescriptor();
        samplersDescriptors.back()->UpdateDescriptorSets(&textureInfos[offset], static_cast<uint32_t>(textureInfos.size()) - offset, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }
}

void Scene::createSamplerDescriptor()
{
    auto& descriptorSetLayout =
        Resource::ResourceManager::GetCurrent()->Shaders[0].GetDescriptors()[DEFAULT_SAMPLER_LAYOUT].GetLayout();
    auto descriptor = new Descriptor(aDevice, 1,
        batchSize,
        1,
        descriptorSetLayout,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        );
    samplersDescriptors.push_back(descriptor);
}

void Scene::updateAll()
{
    if (vertexCount * sizeof(Model::Vertex) > vertexBuffers[nextIndex].GetSize())
    {
        vertexBuffers[nextIndex] = Buffer(aDevice, (vertexCount + 1000) * sizeof(Model::Vertex),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        vertexBuffers[nextIndex].Map();
    }
    if (indexCount * sizeof(Model::Index) > indexBuffers[nextIndex].GetSize())
    {
        indexBuffers[nextIndex] = Buffer(aDevice, (indexCount + 1000) * sizeof(Model::Index),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        indexBuffers[nextIndex].Map();
    }
    CommitBufferUpdate(&vertexBuffers[nextIndex], &indexBuffers[nextIndex]);
    UpdateMeshlets();

    updateSSBODescriptor();
}
