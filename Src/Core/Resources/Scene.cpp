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
    //numOfGroup = 32;
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
    objectBuffers.resize(vertexBuffers.size());

    meshletBuffers.resize(vertexBuffers.size());
    meshletVertexBuffers.resize(meshletBuffers.size());
    meshletIndexBuffers.resize(meshletBuffers.size());
    meshletCullingBuffers.resize(meshletBuffers.size());
    meshletIDBuffers.resize(meshletBuffers.size());
    meshletIDCountBuffers.resize(meshletBuffers.size());
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

        objectBuffers[i] = Buffer(aDevice, (1000) * sizeof(Object),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        objectBuffers[i].Map();

        meshletBuffers[i] = Buffer(aDevice, 100 * sizeof(MeshletInfo),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        meshletBuffers[i].Map();

        meshletVertexBuffers[i] = Buffer(aDevice, 500 * sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        meshletVertexBuffers[i].Map();

        meshletIndexBuffers[i] = Buffer(aDevice, 1000,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        meshletIndexBuffers[i].Map();

        meshletCullingBuffers[i] = Buffer(aDevice, 100 * sizeof(BoundingSphere),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        meshletCullingBuffers[i].Map();

        meshletIDBuffers[i] = Buffer(aDevice, 100 * sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        meshletIDBuffers[i].Map();

        meshletIDCountBuffers[i] = Buffer(aDevice, sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        meshletIDCountBuffers[i].Map();
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
    std::unique_lock<std::mutex> unique_lock(_mutex);
    std::vector<VkDescriptorImageInfo> imageInfos{};
    auto resourceManager = Resource::ResourceManager::GetCurrent();
    for (size_t i = 0; i < count; i++)
    {
        auto& meshInfo = meshInfos[i];
        uint32_t meshId;
        Mesh mesh;
        mesh.transform = meshInfo.transform;
        bool newModel = resourceManager->CreateModel(meshInfo.filePath, meshId);
        auto model = resourceManager->ModelMap[meshId];
        mesh.vertexCount = uint32_t(model->info.vertices.size());
        mesh.indexCount = uint32_t(model->info.indices.size());

        if (newModel)
        {
            modelMap.emplace(meshId, uint32_t(uniqueModels.size()));
            mesh.modelID = uint32_t(uniqueModels.size());
            uniqueModels.push_back({uint32_t(vertexCount), uint32_t(indexCount), meshletCount, model});
            vertexCount += mesh.vertexCount;
            indexCount += mesh.indexCount;
            meshletIndexCount += model->meshletIndexCount;
            meshletVertexCount += model->meshletVertexCount;
            meshletCount += uint32_t(model->meshlets.size());
        }
        else
        {
            mesh.modelID = modelMap[meshId];
        }

        for (uint32_t i = 0, meshletOffset = uniqueModels[mesh.modelID].meshletOffset; i < uint32_t(model->meshlets.size()); i++)
            meshletIDs.push_back({meshletOffset + i, uint32_t(meshes.size())});

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
        
        VkDrawIndexedIndirectCommand drawIndexedCommand;
        drawIndexedCommand.firstInstance = 0;
        drawIndexedCommand.instanceCount = 1;
        drawIndexedCommand.indexCount = mesh.indexCount;
        drawIndexedCommand.firstIndex = uniqueModels[mesh.modelID].indexOffset;
        drawIndexedCommand.vertexOffset = 0;
        static_cast<VkDrawIndexedIndirectCommand*>(drawIndexedIndirectBuffer.GetMappedData())[drawIndexedCommands.size()] = 
            drawIndexedCommand;
        drawIndexedCommands.push_back(drawIndexedCommand);
    }
    if (imageInfos.size())
    {
        appendSamplersDescriptor(imageInfos);
    }
    needUpdate = true;
}

void Scene::Append(std::vector<Model::Vertex>& meshVertices, std::vector<uint32_t>& meshIndices, Transform transform, uint32_t textureId)
{
    std::unique_lock<std::mutex> unique_lock(_mutex);
    std::vector<VkDescriptorImageInfo> imageInfos{};

    Mesh mesh{};
    mesh.transform = transform;
    mesh.vertexCount = static_cast<uint32_t>(meshVertices.size());
    mesh.indexCount = static_cast<uint32_t>(meshIndices.size());
    mesh.textureId = textureId;
    //temporary solution for now
    Model::ModelInfo info{{}, meshVertices, {}, uint32_t(meshIndices.size()), meshIndices};
    auto model = std::make_shared<Model>(info);
    uint32_t meshId;
    Resource::ResourceManager::GetCurrent()->AppendModel(model, meshId);
    
    modelMap.emplace(meshId, uint32_t(uniqueModels.size()));
    mesh.modelID = uint32_t(uniqueModels.size());
    uniqueModels.push_back({uint32_t(vertexCount), uint32_t(indexCount), meshletCount, model});
    vertexCount += mesh.vertexCount;
    indexCount += mesh.indexCount;
    meshletIndexCount += model->meshletIndexCount;
    meshletVertexCount += model->meshletVertexCount;
    meshletCount += uint32_t(model->meshlets.size());

    for (uint32_t i = 0, meshletOffset = uniqueModels[mesh.modelID].meshletOffset; i < uint32_t(model->meshlets.size()); i++)
        meshletIDs.push_back({meshletOffset + i, uint32_t(meshes.size())});

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
    needUpdate = true;
}

void Scene::RemoveAt(uint32_t meshIndex)
{
    meshes.erase(meshes.begin() + meshIndex);
    needUpdate = true;
}

void Scene::RemoveAt(Range removeRange)
{
    for (uint32_t i = 0; i < removeRange.y; i++)
        meshes.erase(meshes.begin() + i + removeRange.x);
    needUpdate = true;
}

void Scene::RemoveAt(std::vector<uint32_t> meshIndices)
{
    for (auto& meshIndex : meshIndices)
        meshes.erase(meshes.begin() + meshIndex);
    needUpdate = true;
}

void Scene::Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex)
{
    if (aDevice->MeshShaderSupport())
    {
        auto& sets = shader.GetDescriptorSets()[bufferIndex];
        shader.GetPipeline().Bind(commandBuffer);
        sets[DEFAULT_MESHLET_LAYOUT] = meshDescriptor->GetSets()[currentBufferIndex];
        sets[DEFAULT_VERTEX_LAYOUT] = vertexDescriptor->GetSets()[currentBufferIndex];
        sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors.front()->GetSets()[0];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            shader.GetPipelineLayout(), 0, static_cast<uint32_t>(sets.size()),
            sets.data(), 0, nullptr);
    }
    else
    {
        auto& sets = shader.GetDescriptorSets()[bufferIndex];
        shader.GetPipeline().Bind(commandBuffer);
        sets[DEFAULT_VERTEX_LAYOUT] = vertexDescriptor->GetSets()[currentBufferIndex];
        sets[DEFAULT_SAMPLER_LAYOUT] = samplersDescriptors.front()->GetSets()[0];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            shader.GetPipelineLayout(), 0, static_cast<uint32_t>(sets.size()),
            sets.data(), 0, nullptr);
    }
}

void Scene::Draw(CommandBuffer& commandBuffer)
{
    vkCmdSetPrimitiveTopology(commandBuffer, Topology);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentBufferIndex].GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexedIndirectCount(commandBuffer, drawIndexedIndirectBuffer.GetBuffer(),
    0, drawIndexedCountBuffer.GetBuffer(),
    0, meshes.size(), sizeof(VkDrawIndexedIndirectCommand));
}

void Scene::DrawIndirect(CommandBuffer& commandBuffer)
{
    if (aDevice->MeshShaderSupport())
    {
        aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawMeshIndirectBuffer.GetBuffer(), 0,
            drawMeshCountBuffer.GetBuffer(),
            0, 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    }
    else
    {
        vkCmdSetPrimitiveTopology(commandBuffer, Topology);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentBufferIndex].GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirectCount(commandBuffer, drawIndexedIndirectBuffer.GetBuffer(),
            0, drawIndexedCountBuffer.GetBuffer(),
            0, meshes.size(), sizeof(VkDrawIndexedIndirectCommand));
    }
}

void Scene::CommitBufferUpdate(Buffer* newVertBuffer, Buffer* newIndexBuffer, Buffer* newObjectBuffer, uint32_t offset, size_t meshOffset)
{
    auto bufferVertices = static_cast<Model::Vertex*>(newVertBuffer->GetMappedData());
    auto bufferIndices = static_cast<Model::Index*>(newIndexBuffer->GetMappedData());
    auto bufferObjects = static_cast<Object*>(newObjectBuffer->GetMappedData());

    Range updateRange = {offset, static_cast<uint32_t>(uniqueModels.size()) - offset};
    applyVertexBufferUpdate(bufferVertices, bufferIndices, updateRange);
    glm::mat4 transform;
    for (size_t i = meshOffset; i < meshes.size(); i++)
    {
        transform = meshes[i].transform.mat4();
        transform[3].w = float(textureIdMap[meshes[i].textureId]);
        bufferObjects[i].transform = transform;
    }
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
    needUpdate = false;
    Resource::ResourceManager::GetCurrent()->TaskPool.Enqueue([this]()
    {
        Resource::ResourceManager::GetCurrent()->TaskPool.Join();
        this->updateAll();
    });
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
    uint32_t minMeshletBufferSize = meshletCount * sizeof(MeshletInfo);
    if (!meshletBuffers[nextIndex].GetBuffer() ||
        meshletBuffers[nextIndex].GetSize() < minMeshletBufferSize)
    {
        meshletBuffers[nextIndex].Resize(minMeshletBufferSize);

        meshletBuffers[nextIndex].Map();

        meshletCullingBuffers[nextIndex].Resize(meshletCount * sizeof(BoundingSphere));
        meshletCullingBuffers[nextIndex].Map();
    }
    if (meshletVertexBuffers[nextIndex].GetSize() < meshletVertexCount * sizeof(uint32_t))
    {
        meshletVertexBuffers[nextIndex].Resize(meshletVertexCount * sizeof(uint32_t));

        meshletVertexBuffers[nextIndex].Map();
    }
    if (meshletIndexBuffers[nextIndex].GetSize() < meshletIndexCount)
    {
        meshletIndexBuffers[nextIndex].Resize(meshletIndexCount);

        meshletIndexBuffers[nextIndex].Map();
    }
    if (meshletIDs.size() * sizeof(MeshletID) > meshletIDBuffers[nextIndex].GetSize())
    {
        meshletIDBuffers[nextIndex].Resize((meshletIDs.size() + 100) * sizeof(MeshletID));
        meshletIDBuffers[nextIndex].Map();
    }

    MeshletInfo* meshletBuffer = static_cast<MeshletInfo*>(meshletBuffers[nextIndex].GetMappedData());
    uint32_t* meshletVertexBuffer = static_cast<uint32_t*>(meshletVertexBuffers[nextIndex].GetMappedData());
    uint8_t* meshletIndexBuffer = static_cast<uint8_t*>(meshletIndexBuffers[nextIndex].GetMappedData());
    BoundingSphere* cullingBuffer = static_cast<BoundingSphere*>(meshletCullingBuffers[nextIndex].GetMappedData());
    uint32_t vertexOffset = 0, indexOffset = 0;
    for (size_t j = 0, i = 0; j < uniqueModels.size(); j++)
    {
        auto& model = uniqueModels[j];
        auto& meshlets = model.model->meshlets;
        for (auto& meshlet : meshlets)
        {
            meshletBuffer[i] = {vertexOffset, indexOffset, meshlet.vertexCount, meshlet.indexCount};
            for (uint32_t i = 0; i < meshlet.vertexCount; i++)
                meshletVertexBuffer[i + vertexOffset] = meshlet.vertices[i] + model.vertexOffset;
            for (uint32_t i = 0; i < meshlet.indexCount; i++)
                meshletIndexBuffer[i + indexOffset] = meshlet.indices[i];

            vertexOffset += meshlet.vertexCount;
            indexOffset += meshlet.indexCount;

            cullingBuffer[i].center = meshlet.center;
            cullingBuffer[i].farVertexID = meshlet.farVertexID;
            cullingBuffer[i].normal = meshlet.normal;
            cullingBuffer[i].coneApex = meshlet.coneApex;
            cullingBuffer[i].cutoff = meshlet.cutoff;
            ++i;
        }
    }
    memcpy(meshletIDBuffers[nextIndex].GetMappedData(), meshletIDs.data(), meshletIDs.size() * sizeof(MeshletID));
    *static_cast<uint32_t*>(meshletIDCountBuffers[nextIndex].GetMappedData()) = uint32_t(meshletIDs.size());
    auto drawMeshTaskCommand = static_cast<glm::uvec3*>(drawMeshIndirectBuffer.GetMappedData());
    uint32_t numofGroup = (static_cast<uint32_t>(meshletIDs.size()) + numOfGroup - 1) / numOfGroup;
    glm::uvec3 groupSize;
    groupSize.x = numofGroup;
    groupSize.y = 1;
    groupSize.z = 1;
    *drawMeshTaskCommand = groupSize;

    //meshletBuffers[nextIndex].Flush();
}

void Scene::UpdateVertexPositions(UniqueModel& model)
{
    auto vertexBufferData = static_cast<Model::Vertex*>(vertexBuffers[currentBufferIndex].GetMappedData());
    for (size_t i = 0; i < model.model->info.vertices.size(); i++)
    {
        auto& vertex = vertexBufferData[model.vertexOffset + i];
        auto& meshVertex = model.model->info.vertices[i];
        vertex.position = meshVertex.position;
    }
}

void Scene::UpdateVertexPositions(Range updateRange)
{
    auto vertexBufferData = static_cast<Model::Vertex*>(vertexBuffers[currentBufferIndex].GetMappedData());
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& model = uniqueModels[updateRange.x + i];
        for (size_t j = 0; i < model.model->info.vertices.size(); j++)
        {
            auto& vertex = vertexBufferData[model.vertexOffset + j];
            auto& meshVertex = model.model->info.vertices[j];
            vertex.position = meshVertex.position;
        }
    }
}

void Scene::UpdateMeshTransform(uint32_t meshIndex)
{
    auto objectBufferData = static_cast<Object*>(objectBuffers[currentBufferIndex].GetMappedData());
    glm::mat4 transform = meshes[meshIndex].transform.mat4();
    transform[3].w = float(meshes[meshIndex].textureId);
    objectBufferData[meshIndex].transform = transform;
}

void Scene::applyVertexBufferUpdate(Model::Vertex* vertexBufferData, Model::Index* indexBufferData, Range& updateRange)
{
    for (uint32_t i = 0; i < updateRange.y; i++)
    {
        auto& model = uniqueModels[i + updateRange.x];
        for (uint32_t j = 0; j < uint32_t(model.model->info.vertices.size()); j++)
        {
            vertexBufferData[model.vertexOffset + j] = model.model->info.vertices[j];
        }
        for (uint32_t j = 0; j < uint32_t(model.model->info.indices.size()); j++)
        {
            indexBufferData[model.indexOffset + j] = model.model->info.indices[j] + model.vertexOffset;
        }
    }
    *static_cast<uint32_t*>(drawIndexedCountBuffer.GetMappedData()) = meshes.size();
    
    vertexBuffers[nextIndex].Flush();
}

void Scene::createIndirectBuffers()
{
    drawIndexedIndirectBuffer = Buffer(aDevice, sizeof(VkDrawIndexedIndirectCommand) * 50,
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

    drawIndexedCountBuffer = Buffer(aDevice, 4,
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    drawIndexedCountBuffer.Map();
    *static_cast<uint32_t*>(drawIndexedCountBuffer.GetMappedData()) = 0;

    drawMeshCountBuffer = Buffer(aDevice, 4,
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    drawMeshCountBuffer.Map();
    *static_cast<uint32_t*>(drawMeshCountBuffer.GetMappedData()) = 1;
    drawMeshCountBuffer.Unmap();
}

void Scene::createSSBODescriptor()
{
    auto& shaders = Resource::ResourceManager::GetCurrent()->Shaders;
    auto& vertexDescriptorSetLayout =
        shaders.front().GetDescriptors()[DEFAULT_VERTEX_LAYOUT].GetLayout();
    vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
        MaxBatchSize * 2,
        2,
        vertexDescriptorSetLayout,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (aDevice->MeshShaderSupport())
    {
        auto& meshDescriptorSetLayout = shaders[5].GetDescriptors()[DEFAULT_MESHLET_LAYOUT].GetLayout();
        meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
            MaxBatchSize * 3 + 1,
            4,
            meshDescriptorSetLayout,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        for (size_t i = 0; i < meshletIDCountBuffers.size(); i++)
        {
            VkDescriptorBufferInfo bufferInfo;
            bufferInfo.buffer = meshletIDCountBuffers[i].GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = meshletIDCountBuffers[i].GetSize();
            aDevice->UpdateDescriptorSet(bufferInfo, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    meshDescriptor->GetSets()[i]);
        }
    }
}

void Scene::updateSSBODescriptor()
{
    //Vertex Buffer
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = vertexBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = vertexBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        vertexDescriptor->GetSets()[nextIndex]);
    //Object Buffer
    bufferInfo.buffer = objectBuffers[nextIndex].GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = objectBuffers[nextIndex].GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        vertexDescriptor->GetSets()[nextIndex]);
    if (aDevice->MeshShaderSupport())
    {
        //Meshlet Buffer
        bufferInfo.buffer = meshletBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);
        bufferInfo.buffer = meshletVertexBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletVertexBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);
        bufferInfo.buffer = meshletIndexBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletIndexBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        meshDescriptor->GetSets()[nextIndex]);

        //Meshlet Boundings Buffer
        bufferInfo.buffer = meshletCullingBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletCullingBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                meshDescriptor->GetSets()[nextIndex]);
        //Meshlet ID Buffer
        bufferInfo.buffer = meshletIDBuffers[nextIndex].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = meshletIDBuffers[nextIndex].GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
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
    std::unique_lock<std::mutex> unique_lock(_mutex);
    if (vertexCount * sizeof(Model::Vertex) > vertexBuffers[nextIndex].GetSize())
    {
        vertexBuffers[nextIndex].Resize((vertexCount + 1000) * sizeof(Model::Vertex));
        vertexBuffers[nextIndex].Map();
    }
    if (indexCount * sizeof(Model::Index) > indexBuffers[nextIndex].GetSize())
    {
        indexBuffers[nextIndex].Resize((indexCount + 1000) * sizeof(Model::Index));
        indexBuffers[nextIndex].Map();
    }
    if (meshes.size() * sizeof(Object) > objectBuffers[nextIndex].GetSize())
    {
        objectBuffers[nextIndex].Resize((meshes.size() + 1000) * sizeof(Object));
        objectBuffers[nextIndex].Map();
    }
    CommitBufferUpdate(&vertexBuffers[nextIndex], &indexBuffers[nextIndex], &objectBuffers[nextIndex]);
    UpdateMeshlets();

    updateSSBODescriptor();
}
