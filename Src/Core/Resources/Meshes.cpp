#include "Headers/Meshes.hpp"
#include "Headers/Buffer.hpp"
#include "Headers/SwapChain.hpp"

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
    if (frameResource.vertexBuffer.GetSize() <= vertexCount * sizeof(Mesh::Vertex))
    {
        frameResource.vertexBuffer = Buffer(aDevice, (vertexCount + 1000) * sizeof(Mesh::Vertex),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.vertexBuffer.Map();
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
    auto vertexBufferData = reinterpret_cast<Mesh::Vertex*>(frameResource.vertexBuffer.GetMappedData());
    auto meshletBufferData = reinterpret_cast<MeshletInfo*>(frameResource.meshletBuffer.GetMappedData());
    auto meshletVertexBufferData = reinterpret_cast<uint32_t*>(frameResource.meshletVertexBuffer.GetMappedData());
    auto meshletIndexBufferData = reinterpret_cast<uint8_t*>(frameResource.meshletIndexBuffer.GetMappedData());
    auto meshletCullingBufferData = reinterpret_cast<BoundingSphere*>(frameResource.meshletCullingBuffer.GetMappedData());

    for (auto& id : loadedSet)
    {
        auto mesh = MeshMap[id];

        for (size_t i = 0; i < mesh->data.vertices.size(); i++)
            vertexBufferData[vertexOffset + i] = mesh->data.vertices[i];

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
        vertexOffset += mesh->data.vertices.size();
        indexOffset += mesh->data.indices.size();
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
