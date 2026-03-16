#include "Headers/Meshes.hpp"
#include "Headers/Buffer.hpp"
#include "Headers/SwapChain.hpp"
#include "Resources/Headers/Model.hpp"

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

void Meshes::Load(const char* filePath)
{
    uint32_t id;
    Create(filePath, id);
    Load(id);
}

void Meshes::Load(const uint32_t id)
{
    if (loadedSet.find(id) != loadedSet.end())
    {
        return;
    }

    loadedSet.emplace(id);
}

void Meshes::initFrameResource(MeshFrameResource& frameResource)
{
    if (frameResource.vertexBuffer.GetSize() < vertexCount * sizeof(Model::Vertex))
    {
        frameResource.vertexBuffer = Buffer(aDevice, (vertexCount + 1000) * sizeof(Model::Vertex),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.vertexBuffer.Map();
    }

    if (frameResource.indexBuffer.GetSize() < indexCount * sizeof(Model::Index))
    {
        frameResource.indexBuffer = Buffer(aDevice, (indexCount + 1000) * sizeof(Model::Index),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.indexBuffer.Map();
    }

    if (frameResource.meshletBuffer.GetSize() < meshletCount * sizeof(MeshletInfo))
    {
        frameResource.meshletBuffer = Buffer(aDevice, (meshletCount + 100) * sizeof(MeshletInfo),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletBuffer.Map();
    }

    if (frameResource.meshletVertexBuffer.GetSize() < meshletVertexCount * sizeof(uint32_t))
    {
        frameResource.meshletVertexBuffer = Buffer(aDevice, (meshletVertexCount + 500) * sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletVertexBuffer.Map();
    }

    if (frameResource.meshletIndexBuffer.GetSize() < meshletIndexCount)
    {
        frameResource.meshletIndexBuffer = Buffer(aDevice, meshletIndexCount + 1000,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        frameResource.meshletIndexBuffer.Map();
    }

    if (frameResource.meshletCullingBuffer.GetSize() < meshletCount * sizeof(BoundingSphere))
    {
        frameResource.meshletCullingBuffer = Buffer(aDevice, (meshletCount + 100) * sizeof(BoundingSphere),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.meshletCullingBuffer.Map();
    }

    if (frameResource.meshletIDCountBuffer.GetSize() < sizeof(uint32_t))
    {
        frameResource.meshletIDCountBuffer = Buffer(aDevice, sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        frameResource.meshletIDCountBuffer.Map();
    }
}

void Meshes::prepareFrameResources()
{
    uint bufferIndex = currentBufferIndex;
    if (vertexCount * sizeof(Model::Vertex) <
        frameResources[currentBufferIndex].vertexBuffer.GetSize() ||

        indexCount * sizeof(Model::Index) <
        frameResources[currentBufferIndex].indexBuffer.GetSize() ||

        meshletVertexCount * sizeof(uint32_t) <
        frameResources[currentBufferIndex].meshletVertexBuffer.GetSize() ||

        indexCount <
        frameResources[currentBufferIndex].meshletIndexBuffer.GetSize() ||

        meshletCount * sizeof(MeshFrameResource) <
        frameResources[currentBufferIndex].meshletBuffer.GetSize() ||

        meshletCount * sizeof(BoundingSphere) <
        frameResources[currentBufferIndex].meshletCullingBuffer.GetSize()
    )
    {
        bufferIndex = NextFrameIndex(bufferIndex);
        initFrameResource(frameResources[bufferIndex]);
    }
    *reinterpret_cast<uint32_t*>(frameResources[bufferIndex].meshletIDCountBuffer.GetMappedData()) = meshletIDCount;
}
