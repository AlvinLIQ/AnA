#include "Headers/Meshes.hpp"
#include "Headers/Buffer.hpp"
#include "Headers/SwapChain.hpp"
#include "vulkan/vulkan_core.h"
#include <memory>

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

bool Meshes::Create(const char* filePath, uint32_t& id, uint32_t& count)
{
    auto iter = MeshPathIndexMap.find(filePath);
    if (iter != MeshPathIndexMap.end())
    {
        id = iter->second;
        count = 1;
        return false;
    }
    std::vector<Mesh::MeshData> meshDatas;
    Mesh::CreateMeshesFromFile(filePath, meshDatas);
    count = uint32_t(meshDatas.size());
    MeshPathIndexMap.emplace(filePath, meshId);
    id = meshId;

    for (auto& meshData : meshDatas)
        MeshMap.emplace(meshId++, std::make_shared<Mesh>(std::move(meshData)));

    return true;
}

bool Meshes::Create(std::shared_ptr<AnA::Mesh> mesh, uint32_t& id)
{
    id = meshId;
    MeshMap.emplace(meshId++, mesh);

    return true;
}

void Meshes::Load(const char* filePath, uint32_t& id, uint32_t& count)
{
    Create(filePath, id, count);
    for (uint32_t i = 0; i < count; i++)
        Load(id + i);
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
    vertexCount += (mesh->data.vertices.size());
    indexCount += (mesh->data.indices.size());
    meshletCount += uint32_t(mesh->meshlets.size());
    needUpdate = true;
}

void Meshes::Append(uint32_t id, std::vector<AnA::Mesh::Vertex>& vertices)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (MeshMap.find(id) == MeshMap.end())
        return;

    auto mesh = MeshMap[id];
    mesh->data.vertices.insert(mesh->data.vertices.end(), vertices.begin(), vertices.end());
    if (loadedSet.find(id) == loadedSet.end())
        return;

    needUpdate = true;
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
    if (frameResource.meshletBuffer.GetSize() <= meshletCount * sizeof(Mesh::Meshlet))
    {
        frameResource.meshletBuffer = Buffer(aDevice, (meshletCount + 100) * sizeof(Mesh::Meshlet),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletBuffer.Map();
    }
}

void Meshes::rebuildFrameResource(MeshFrameResource& frameResource)
{
    uint32_t meshletOffset = 0;
    auto meshletBufferData = reinterpret_cast<Mesh::Meshlet*>(frameResource.meshletBuffer.GetMappedData());

    for (auto& id : loadedSet)
    {
        auto mesh = MeshMap[id];
        mesh->Load(aDevice);

        mesh->meshletOffset = meshletOffset;
        memcpy(&meshletBufferData[meshletOffset], mesh->meshlets.data(), mesh->meshlets.size() * sizeof(mesh->meshlets[0]));
        meshletOffset += uint32_t(mesh->meshlets.size());
    }
}

uint32_t Meshes::prepareFrameResources()
{
    uint32_t bufferIndex = currentBufferIndex;
    if (meshletCount * sizeof(Mesh::Meshlet) >
        frameResources[currentBufferIndex].meshletBuffer.GetSize()
    )
    {
        bufferIndex = NextFrameIndex(bufferIndex);
        initFrameResource(frameResources[bufferIndex]);
    }
    rebuildFrameResource(frameResources[bufferIndex]);

    return bufferIndex;
}
