#include "Headers/Meshes.hpp"
#include "Headers/Buffer.hpp"
#include "Headers/SwapChain.hpp"
#include "vulkan/vulkan_core.h"

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

}

void Meshes::Init()
{

}

bool Meshes::Create(const char* filePath, uint32_t& id)
{
    auto iter = MeshPathIndexMap.find(filePath);
    if (iter != MeshPathIndexMap.end())
    {
        id = iter->second;
        return false;
    }
    std::shared_ptr<Mesh> mesh;
    Mesh::CreateMeshFromFile(filePath, mesh);
    MeshPathIndexMap.emplace(filePath, meshId);
    id = meshId;
    MeshMap.emplace(meshId++, mesh);

    return true;
}

bool Meshes::Create(std::shared_ptr<AnA::Mesh> mesh, uint32_t& id)
{
    id = meshId;
    MeshMap.emplace(meshId++, mesh);

    return true;
}

void Meshes::Load(const char* filePath, uint32_t& id)
{
    Create(filePath, id);
    Load(id);
}

void Meshes::Load(std::shared_ptr<Mesh> mesh, uint32_t& id)
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
    vertexCount += (mesh->data.vertices.size());
    indexCount += (mesh->data.indices.size());
    meshletCount += uint32_t(mesh->meshlets.size());
    meshletVertexCount += uint32_t(mesh->meshletVertexCount);
    meshletIndexCount += uint32_t(mesh->meshletIndexCount);
    needUpdate = true;
}

void Meshes::Append(uint32_t id, std::vector<AnA::Mesh::Vertex>& vertices)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (MeshMap.find(id) == MeshMap.end())
        return;

    auto mesh = MeshMap[id];
    bool isLastMesh = mesh->vertexOffset + uint32_t(mesh->data.vertices.size()) == vertexCount;
    mesh->data.vertices.insert(mesh->data.vertices.end(), vertices.begin(), vertices.end());
    if (loadedSet.find(id) == loadedSet.end())
        return;

    if (isLastMesh &&
        frameResources[currentBufferIndex].vertexBuffer.GetSize() >
        (vertexCount + uint32_t(vertices.size())) * sizeof(Mesh::Vertex))
    {
        auto vertexBufferData =
            reinterpret_cast<Mesh::Vertex*>(frameResources[currentBufferIndex].vertexBuffer.GetMappedData());
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
    if (frameResource.vertexBuffer.GetSize() <= loadedSet.size() * sizeof(VkDeviceSize))
    {
        frameResource.vertexBuffer = Buffer(aDevice, (loadedSet.size() + 10) * sizeof(VkDeviceSize),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.vertexBuffer.Map();
    }

    if (frameResource.meshletBuffer.GetSize() <= meshletCount * sizeof(Mesh::MeshletInfo))
    {
        frameResource.meshletBuffer = Buffer(aDevice, (meshletCount + 100) * sizeof(Mesh::MeshletInfo),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletBuffer.Map();
    }
}

void Meshes::rebuildFrameResource(MeshFrameResource& frameResource)
{
    uint32_t meshletOffset = 0, vertexOffset = 0;
    auto vertexBufferData = reinterpret_cast<VkDeviceAddress*>(frameResource.vertexBuffer.GetMappedData());
    auto meshletBufferData = reinterpret_cast<Mesh::MeshletInfo*>(frameResource.meshletBuffer.GetMappedData());

    for (auto& id : loadedSet)
    {
        auto mesh = MeshMap[id];
        mesh->meshletOffset = meshletOffset;
        if (!mesh->loaded)
            mesh->Load(aDevice, meshletBufferData, meshletOffset);
        else
            meshletOffset += uint32_t(mesh->meshlets.size());

        vertexBufferData[vertexOffset++] = mesh->vertexBuffer.GetAddress();
    }
}

uint32_t Meshes::prepareFrameResources()
{
    uint32_t bufferIndex = currentBufferIndex;
    if (vertexCount * sizeof(Mesh::Vertex) >
        frameResources[currentBufferIndex].vertexBuffer.GetSize() ||

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

    return bufferIndex;
}
