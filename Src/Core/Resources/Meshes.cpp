#include "Headers/Meshes.hpp"
#include "Headers/Buffer.hpp"
#include "Headers/SwapChain.hpp"
#include "Headers/ResourceManager.hpp"

using namespace AnA;
using namespace Resources;

uint32_t meshId = 0;

Meshes::Meshes(Device* mDevice) : aDevice {mDevice}
{
    for (auto& frameResource : frameResources)
        initFrameResource(frameResource);
}

Meshes::~Meshes()
{
    if (vertexDescriptor)
        delete vertexDescriptor;
    if (meshDescriptor != nullptr)
        delete meshDescriptor;
}

void Meshes::Init()
{
    auto resourceManager = Resources::ResourceManager::GetCurrent();
    if (vertexDescriptor == nullptr)
    {
        vertexDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
                MAX_FRAMES_IN_FLIGHT * 1,
                1, resourceManager->Shaders.front().GetDescriptors()[DEFAULT_VERTEX_LAYOUT].GetLayout(),
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
    if (aDevice->MeshShaderSupport() && meshDescriptor == nullptr)
    {
        auto& meshDescriptorSetLayout = resourceManager->Shaders[MESH_PIPELINE_ID].GetDescriptors()[DEFAULT_MESHLET_LAYOUT].GetLayout();
        meshDescriptor = new Descriptor(aDevice, MAX_FRAMES_IN_FLIGHT,
            MAX_FRAMES_IN_FLIGHT * 4,
            4,
            meshDescriptorSetLayout,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        updateDescriptors(i);
        frameResources[i].vertexDescriptorSet = vertexDescriptor->GetSets()[i];
        if (meshDescriptor)
            frameResources[i].meshDescriptorSet = meshDescriptor->GetSets()[i];
    }
}

bool Meshes::Create(const char* filePath, uint32_t& id)
{
    auto iter = MeshPathIndexMap.find(filePath);
    if (iter != MeshPathIndexMap.end())
    {
        id = iter->second;
        return false;
    }
    std::shared_ptr<Model> mesh;
    Model::CreateModelFromFile(filePath, mesh);
    MeshPathIndexMap.emplace(filePath, meshId);
    id = meshId;
    MeshMap.emplace(meshId++, mesh);

    return true;
}

bool Meshes::Create(std::shared_ptr<AnA::Model> model, uint32_t& id)
{
    id = meshId;
    MeshMap.emplace(meshId++, model);

    return true;
}

void Meshes::Load(const char* filePath, uint32_t& id)
{
    Create(filePath, id);
    Load(id);
}

void Meshes::Load(std::shared_ptr<Model> mesh, uint32_t& id)
{
    id = meshId;
    MeshMap.emplace(meshId++, mesh);
    Load(id);
}

void Meshes::Load(const uint32_t id)
{
    if (loadedSet.find(id) != loadedSet.end())
    {
        return;
    }
    loadedSet.emplace(id);
    auto mesh = MeshMap[id];
    mesh->vertexOffset = uint32_t(vertexCount);
    mesh->indexOffset = uint32_t(indexCount);
    mesh->meshletOffset = meshletCount;
    vertexCount += (mesh->info.vertices.size());
    indexCount += (mesh->info.indices.size());
    meshletCount += uint32_t(mesh->meshlets.size());
    meshletVertexCount += uint32_t(mesh->meshletVertexCount);
    meshletIndexCount += uint32_t(mesh->meshletIndexCount);
    needUpdate = true;
}

void Meshes::Append(uint32_t id, std::vector<AnA::Model::Vertex>& vertices)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (MeshMap.find(id) == MeshMap.end())
        return;

    auto mesh = MeshMap[id];
    bool isLastMesh = mesh->vertexOffset + uint32_t(mesh->info.vertices.size()) == vertexCount;
    mesh->info.vertices.insert(mesh->info.vertices.end(), vertices.begin(), vertices.end());
    if (loadedSet.find(id) == loadedSet.end())
        return;

    if (isLastMesh &&
        frameResources[currentBufferIndex].vertexBuffer.GetSize() >
        (vertexCount + uint32_t(vertices.size())) * sizeof(Model::Vertex))
    {
        auto vertexBufferData =
            reinterpret_cast<Model::Vertex*>(frameResources[currentBufferIndex].vertexBuffer.GetMappedData());
        for (uint32_t i = 0; i < uint32_t(vertices.size()); i++)
        {
            vertexBufferData[i + vertexCount] = vertices[i];
        }
    }
    else
    {
        needUpdate = true;
    }
    vertexCount += uint32_t(vertices.size());
}

void Meshes::Update()
{
    std::unique_lock<std::mutex> lock(_mutex);
    currentBufferIndex = prepareFrameResources();
    needUpdate = false;
}

void Meshes::initFrameResource(MeshFrameResource& frameResource)
{
    if (frameResource.vertexBuffer.GetSize() <= vertexCount * sizeof(Model::Vertex))
    {
        frameResource.vertexBuffer = Buffer(aDevice, (vertexCount + 1000) * sizeof(Model::Vertex),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.vertexBuffer.Map();
    }

    if (frameResource.indexBuffer.GetSize() <= indexCount * sizeof(Model::Index))
    {
        frameResource.indexBuffer = Buffer(aDevice, (indexCount + 1000) * sizeof(Model::Index),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.indexBuffer.Map();
    }

    if (frameResource.meshletBuffer.GetSize() <= meshletCount * sizeof(MeshletInfo))
    {
        frameResource.meshletBuffer = Buffer(aDevice, (meshletCount + 100) * sizeof(MeshletInfo),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletBuffer.Map();
    }

    if (frameResource.meshletVertexBuffer.GetSize() <= meshletVertexCount * sizeof(uint32_t))
    {
        frameResource.meshletVertexBuffer = Buffer(aDevice, (meshletVertexCount + 500) * sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletVertexBuffer.Map();
    }

    if (frameResource.meshletIndexBuffer.GetSize() <= meshletIndexCount)
    {
        frameResource.meshletIndexBuffer = Buffer(aDevice, meshletIndexCount + 1000,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletIndexBuffer.Map();
    }

    if (frameResource.meshletCullingBuffer.GetSize() <= meshletCount * sizeof(BoundingSphere))
    {
        frameResource.meshletCullingBuffer = Buffer(aDevice, (meshletCount + 100) * sizeof(BoundingSphere),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.meshletCullingBuffer.Map();
    }
}

void Meshes::rebuildFrameResource(MeshFrameResource& frameResource)
{
    size_t vertexOffset = 0, indexOffset = 0, meshletOffset = 0;
    uint32_t meshletVertexOffset = 0, meshletIndexOffset = 0;
    auto vertexBufferData = reinterpret_cast<Model::Vertex*>(frameResource.vertexBuffer.GetMappedData());
    auto indexBufferData = reinterpret_cast<Model::Index*>(frameResource.indexBuffer.GetMappedData());
    auto meshletBufferData = reinterpret_cast<MeshletInfo*>(frameResource.meshletBuffer.GetMappedData());
    auto meshletVertexBufferData = reinterpret_cast<uint32_t*>(frameResource.meshletVertexBuffer.GetMappedData());
    auto meshletIndexBufferData = reinterpret_cast<uint8_t*>(frameResource.meshletIndexBuffer.GetMappedData());
    auto meshletCullingBufferData = reinterpret_cast<BoundingSphere*>(frameResource.meshletCullingBuffer.GetMappedData());

    for (auto& id : loadedSet)
    {
        auto mesh = MeshMap[id];

        for (size_t i = 0; i < mesh->info.vertices.size(); i++)
            vertexBufferData[vertexOffset + i] = mesh->info.vertices[i];
        for (size_t i = 0; i < mesh->info.indices.size(); i++)
            indexBufferData[indexOffset + i] = mesh->info.indices[i] + Model::Index(vertexOffset);

        mesh->meshletOffset = uint32_t(meshletOffset);
        for (auto& meshlet : mesh->meshlets)
        {
            meshletBufferData[meshletOffset] =
                {uint32_t(meshletVertexOffset), uint32_t(meshletIndexOffset),
                    meshlet.vertexCount, meshlet.indexCount};
            for (uint32_t j = 0; j < meshlet.vertexCount; j++)
                meshletVertexBufferData[meshletVertexOffset + j] = meshlet.vertices[j] + uint32_t(vertexOffset);
            for (uint32_t j = 0; j < meshlet.indexCount; j++)
                meshletIndexBufferData[meshletIndexOffset + j] = meshlet.indices[j];

            meshletVertexOffset += meshlet.vertexCount;
            meshletIndexOffset += meshlet.indexCount;

            meshletCullingBufferData[meshletOffset++] = {meshlet.center, meshlet.radius,
                meshlet.normal, meshlet.cutoff,
                meshlet.coneApex, 0.0f};
        }
        mesh->vertexOffset = uint32_t(vertexOffset);
        mesh->indexOffset = uint32_t(indexOffset);
        vertexOffset += mesh->info.vertices.size();
        indexOffset += mesh->info.indices.size();
    }
}

void Meshes::updateDescriptors(uint32_t bufferIndex)
{
    //Vertex Buffer
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = frameResources[bufferIndex].vertexBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = frameResources[bufferIndex].vertexBuffer.GetSize();
    aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        vertexDescriptor->GetSets()[bufferIndex]);
    frameResources[bufferIndex].vertexDescriptorSet = vertexDescriptor->GetSets()[bufferIndex];
    if(aDevice->MeshShaderSupport())
    {
        //Meshlet Buffer
        bufferInfo.buffer = frameResources[bufferIndex].meshletBuffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = frameResources[bufferIndex].meshletBuffer.GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                meshDescriptor->GetSets()[bufferIndex]);
        bufferInfo.buffer = frameResources[bufferIndex].meshletVertexBuffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = frameResources[bufferIndex].meshletVertexBuffer.GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                meshDescriptor->GetSets()[bufferIndex]);
        bufferInfo.buffer = frameResources[bufferIndex].meshletIndexBuffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = frameResources[bufferIndex].meshletIndexBuffer.GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                meshDescriptor->GetSets()[bufferIndex]);

        //Meshlet Boundings Buffer
        bufferInfo.buffer = frameResources[bufferIndex].meshletCullingBuffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = frameResources[bufferIndex].meshletCullingBuffer.GetSize();
        aDevice->UpdateDescriptorSet(bufferInfo, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                meshDescriptor->GetSets()[bufferIndex]);
    }
}

uint32_t Meshes::prepareFrameResources()
{
    uint32_t bufferIndex = currentBufferIndex;
    if (vertexCount * sizeof(Model::Vertex) >
        frameResources[currentBufferIndex].vertexBuffer.GetSize() ||

        indexCount * sizeof(Model::Index) >
        frameResources[currentBufferIndex].indexBuffer.GetSize() ||

        meshletVertexCount * sizeof(uint32_t) >
        frameResources[currentBufferIndex].meshletVertexBuffer.GetSize() ||

        indexCount >
        frameResources[currentBufferIndex].meshletIndexBuffer.GetSize() ||

        meshletCount * sizeof(MeshFrameResource) >
        frameResources[currentBufferIndex].meshletBuffer.GetSize() ||

        meshletCount * sizeof(BoundingSphere) >
        frameResources[currentBufferIndex].meshletCullingBuffer.GetSize()
    )
    {
        bufferIndex = NextFrameIndex(bufferIndex);
        initFrameResource(frameResources[bufferIndex]);
    }
    rebuildFrameResource(frameResources[bufferIndex]);
    updateDescriptors(bufferIndex);

    return bufferIndex;
}
