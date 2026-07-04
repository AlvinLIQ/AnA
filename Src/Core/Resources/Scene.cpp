#include "Headers/Scene.hpp"
#include "Headers/Device.hpp"
#include "Headers/ResourceManager.hpp"
#include "Headers/Types.hpp"
#include <memory>

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
}

void Scene::Init()
{
    objectBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    objectDataBuffers.resize(objectBuffers.size());
    collisionBuffer.resize(objectBuffers.size());

    meshletIDBuffers.resize(objectBuffers.size());
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        objectBuffers[i] = Buffer(aDevice, 1000 * sizeof(Object),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        objectBuffers[i].Map();

        objectDataBuffers[i] = Buffer(aDevice, sizeof(ObjectData),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        objectDataBuffers[i].Map();

        collisionBuffer[i] = Buffer(aDevice, 1000 * sizeof(CollisionData),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        collisionBuffer[i].Map();

        meshletIDBuffers[i] = Buffer(aDevice, 100 * sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        meshletIDBuffers[i].Map();
    }
    //createBuffers();
    createIndirectBuffers();
}

void Scene::Append(const std::vector<MeshInfo>& meshInfos)
{
    Append(meshInfos.data(), meshInfos.size());
}

void Scene::Append(const MeshInfo* meshInfos, size_t count)
{
    std::unique_lock<std::mutex> unique_lock(_mutex);
    auto resourceManager = Resources::ResourceManager::GetCurrent();
    for (size_t i = 0; i < count; i++)
    {
        auto& meshInfo = meshInfos[i];
        MeshObject meshObj;
        meshObj.transform = meshInfo.transform;
        resourceManager->Meshes.Load(meshInfo.filePath, meshObj.meshId);
        auto mesh = resourceManager->Meshes.MeshMap[meshObj.meshId];
        meshObj.vertexCount = uint32_t(mesh->data.vertices.size());
        meshObj.indexCount = uint32_t(mesh->data.indices.size());
        meshObj.textureId = meshInfo.textureId;

        meshletIDCount += uint32_t(mesh->meshlets.size());

        meshes.push_back(meshObj);
        if (this->MeshAppend)
            this->MeshAppend(meshInfo.filePath, uint32_t(meshes.size()) - 1);

        VkDrawIndexedIndirectCommand drawIndexedCommand;
        drawIndexedCommand.firstInstance = 0;
        drawIndexedCommand.instanceCount = 1;
        drawIndexedCommand.indexCount = meshObj.indexCount;
        drawIndexedCommand.firstIndex = mesh->indexOffset;
        drawIndexedCommand.vertexOffset = 0;
        static_cast<VkDrawIndexedIndirectCommand*>(drawIndexedIndirectBuffer.GetMappedData())[drawIndexedCommands.size()] =
            drawIndexedCommand;
        drawIndexedCommands.push_back(drawIndexedCommand);
    }
    needUpdate = true;
}

void Scene::Append(std::vector<Mesh::Vertex>& meshVertices, std::vector<uint32_t>& meshIndices, Transform transform, uint32_t textureId)
{
    std::unique_lock<std::mutex> unique_lock(_mutex);

    MeshObject meshObj{};
    meshObj.transform = transform;
    meshObj.vertexCount = static_cast<uint32_t>(meshVertices.size());
    meshObj.indexCount = static_cast<uint32_t>(meshIndices.size());
    meshObj.textureId = textureId;
    //temporary solution for now
    Mesh::MeshData data{{}, std::move(meshVertices), {}, {}, uint32_t(meshIndices.size()), std::move(meshIndices), ""};
    auto model = std::make_shared<Mesh>(data);
    Resources::ResourceManager::GetCurrent()->Meshes.Load(model, meshObj.meshId);

    meshletIDCount += uint32_t(model->meshlets.size());

    meshes.push_back(meshObj);

    needUpdate = true;
}

void Scene::RemoveAt(uint32_t meshIndex)
{
    //auto& modelMap = Resources::ResourceManager::GetCurrent()->Meshes.MeshMap;
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

void Scene::Bind(CommandBuffer& commandBuffer, Shader& shader)
{
    auto aResourceManager = Resources::ResourceManager::GetCurrent();
    shader.GetPipeline().Bind(commandBuffer);

    uint32_t bufferIndex = 0;
    VkDeviceSize offset = 0;
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader.GetPipelineLayout(), 0, 1, &bufferIndex, &offset);

    auto& frameResource = aResourceManager->Meshes.GetCurrentFrameResource();

    meshPushConstant.projView = aResourceManager->MainCamera.GetProjectionMatrix()
        * aResourceManager->MainCamera.GetView();
    meshPushConstant.vertexPtr = frameResource.vertexBuffer.GetAddress();
    meshPushConstant.objectPtr = objectBuffers[currentBufferIndex].GetAddress();
    meshPushConstant.miscPtr = aResourceManager->GetMiscBufferAddress();
    meshPushConstant.meshletIDPtr = meshletIDBuffers[currentBufferIndex].GetAddress();
    meshPushConstant.meshletPtr = frameResource.meshletBuffer.GetAddress();
    meshPushConstant.meshVertexPtr = frameResource.meshletVertexBuffer.GetAddress();
    meshPushConstant.meshIndexPtr = frameResource.meshletIndexBuffer.GetAddress();
    meshPushConstant.meshletCullingPtr = frameResource.meshletCullingBuffer.GetAddress();
    vkCmdPushConstants(commandBuffer, shader.GetPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT |
        VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(meshPushConstant),
        &meshPushConstant);

    aDevice->vkCmdSetPolygonModeEXT(commandBuffer, PolygonMode);
}

void Scene::Draw(CommandBuffer& commandBuffer)
{
    vkCmdSetPrimitiveTopology(commandBuffer, Topology);
    if (PolygonMode == VK_POLYGON_MODE_POINT || Topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
    {
        vkCmdDraw(commandBuffer, Resources::ResourceManager::GetCurrent()->Meshes.GetVertexCount(), 1, 0, 0);
    }
}

void Scene::DrawIndirect(CommandBuffer& commandBuffer)
{
    aDevice->vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, drawMeshIndirectBuffers[currentBufferIndex].GetBuffer(), 0,
        drawMeshCountBuffer.GetBuffer(),
        0, 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
}

void Scene::CommitBufferUpdate(Buffer* newObjectBuffer, size_t meshOffset)
{
    auto bufferObjects = static_cast<Object*>(newObjectBuffer->GetMappedData());
    auto bufferDrawIndexedIndirect = static_cast<VkDrawIndexedIndirectCommand*>(drawIndexedIndirectBuffer.GetMappedData());

    *static_cast<uint32_t*>(drawIndexedCountBuffer.GetMappedData()) = uint32_t(meshes.size());
    auto& modelMap = Resources::ResourceManager::GetCurrent()->Meshes.MeshMap;
    glm::mat4 transform;
    for (size_t i = meshOffset; i < meshes.size(); i++)
    {
        transform = meshes[i].transform.mat4();
        auto& model = modelMap[meshes[i].meshId];
        bufferObjects[i].halfVolume = glm::vec4(glm::mat3(transform) * ((model->data.maxBounding - model->data.minBounding) * 0.5f), 1.0f);
        bufferObjects[i].center = transform * glm::vec4(model->center, 1.0f);
        auto& scale = meshes[i].transform.scale;
        bufferObjects[i].radius = model->radius * std::max(scale.x, std::max(scale.y, scale.z));
        bufferObjects[i].transform = transform;

        bufferDrawIndexedIndirect[i].indexCount = meshes[i].indexCount;
        bufferDrawIndexedIndirect[i].instanceCount = 1;
        bufferDrawIndexedIndirect[i].firstIndex = model->indexOffset;
        bufferDrawIndexedIndirect[i].vertexOffset = 0;
        bufferDrawIndexedIndirect[i].firstInstance = 0;
    }
}

void Scene::Update()
{
    needUpdate = false;
    auto& taskPool = Resources::ResourceManager::GetCurrent()->TaskPool;
    taskPool.Enqueue([this]()
    {
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
    uint32_t i = 0;
    auto& modelMap = Resources::ResourceManager::GetCurrent()->Meshes.MeshMap;
    if (meshletIDCount * sizeof(MeshletID) > meshletIDBuffers[nextIndex].GetSize())
    {
        meshletIDBuffers[nextIndex].Resize((meshletIDCount + 100) * sizeof(MeshletID));
        meshletIDBuffers[nextIndex].Map();
    }
    MeshletID* meshletIDBuffer = static_cast<MeshletID*>(meshletIDBuffers[nextIndex].GetMappedData());
    //Update meshlet id buffer
    i = 0;
    for (uint32_t meshID = 0, meshletID; meshID < uint32_t(meshes.size()); meshID++)
    {
        auto& model = modelMap[meshes[meshID].meshId];
        for (meshletID = 0; meshletID < uint32_t(model->meshlets.size()); meshletID++)
            meshletIDBuffer[i++] = {meshletID + model->meshletOffset, meshID};
    }
    meshletIDCount = i;

    auto drawMeshTaskCommand = static_cast<VkDrawMeshTasksIndirectCommandEXT*>(drawMeshIndirectBuffers[nextIndex].GetMappedData());
    uint32_t numofGroup = (meshletIDCount + numOfGroup - 1) / numOfGroup;
    drawMeshTaskCommand->groupCountX = numofGroup;

    //meshletBuffers[nextIndex].Flush();
}

void Scene::UpdateMeshTransform(uint32_t meshIndex)
{
    auto objectBufferData = static_cast<Object*>(objectBuffers[currentBufferIndex].GetMappedData());
    glm::mat4 transform = meshes[meshIndex].transform.mat4();
    auto& model = Resources::ResourceManager::GetCurrent()->Meshes.MeshMap[meshes[meshIndex].meshId];
    objectBufferData[meshIndex].halfVolume = glm::vec4(glm::mat3(transform) * ((model->data.maxBounding - model->data.minBounding) * 0.5f), 1.0f);
    objectBufferData[meshIndex].center = transform * glm::vec4(model->center, 1.0f);
    auto& scale = meshes[meshIndex].transform.scale;
    objectBufferData[meshIndex].radius = model->radius * std::max(scale.x, std::max(scale.y, scale.z));
    objectBufferData[meshIndex].transform = transform;
}

void Scene::createIndirectBuffers()
{
    drawIndexedIndirectBuffer = Buffer(aDevice, sizeof(VkDrawIndexedIndirectCommand) * MaxBatchSize,
    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    drawIndexedIndirectBuffer.Map();
    auto drawIndexedIndirectCommand =
        static_cast<VkDrawIndexedIndirectCommand*>(drawIndexedIndirectBuffer.GetMappedData());
    drawIndexedIndirectCommand->firstIndex = 0;
    drawIndexedIndirectCommand->firstInstance = 0;
    drawIndexedIndirectCommand->instanceCount = 1;
    drawIndexedIndirectCommand->vertexOffset = 0;

    drawMeshIndirectBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (auto& drawMeshIndirectBuffer : drawMeshIndirectBuffers)
    {
        drawMeshIndirectBuffer = Buffer(aDevice, sizeof(VkDrawMeshTasksIndirectCommandEXT),
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        drawMeshIndirectBuffer.Map();
        auto drawMeshIndirectCommand = static_cast<VkDrawMeshTasksIndirectCommandEXT*>(drawMeshIndirectBuffer.GetMappedData());
        drawMeshIndirectCommand->groupCountX = 0;
        drawMeshIndirectCommand->groupCountY = 1;
        drawMeshIndirectCommand->groupCountZ = 1;
    }

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

void Scene::updateAll()
{
    std::unique_lock<std::mutex> unique_lock(_mutex);
    if (meshes.size() * sizeof(Object) > objectBuffers[nextIndex].GetSize())
    {
        objectBuffers[nextIndex].Resize((meshes.size() + 1000) * sizeof(Object));
        objectBuffers[nextIndex].Map();
    }
    //Resources::ResourceManager::GetCurrent()->Meshes.Update();
    CommitBufferUpdate(&objectBuffers[nextIndex]);
    UpdateMeshlets();

    objectCount = uint32_t(meshes.size());
    meshletCount = Resources::ResourceManager::GetCurrent()->Meshes.GetMeshletCount();

    currentBufferIndex = nextIndex;
    nextIndex = NextFrameIndex(nextIndex);
    commandBufferNeedUpdate = true;
}
