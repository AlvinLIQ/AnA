#include "Headers/Meshes.hpp"
#include "Headers/Buffer.hpp"
#include "Headers/SwapChain.hpp"
#include "Resources/Headers/Model.hpp"
#include <memory>
#include <strings.h>

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
    needUpdate = true;
}

void Meshes::Update()
{
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
    size_t vertexOffset = 0, indexOffset = 0, meshletOffset = 0, meshletVertexOffset = 0, meshletIndexOffset = 0;
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

        for (auto& meshlet : mesh->meshlets)
        {
            meshletBufferData[meshletOffset] =
                {uint32_t(vertexOffset), uint32_t(indexOffset),
                    meshlet.vertexCount, meshlet.indexCount};
            for (auto& meshletVertex : meshlet.vertices)
                meshletVertexBufferData[meshletVertexOffset++] = meshletVertex + uint32_t(vertexOffset);
            for (auto& meshletIndex : meshlet.indices)
                meshletIndexBufferData[meshletIndexOffset++] = meshletIndex;

            meshletCullingBufferData[meshletOffset++] = {meshlet.center, meshlet.radius,
                meshlet.normal, meshlet.cutoff,
                meshlet.coneApex, 0.0f};
        }
        vertexOffset += mesh->info.vertices.size();
        indexOffset += mesh->info.indices.size();
    }
}

uint32_t Meshes::prepareFrameResources()
{
    uint32_t bufferIndex = currentBufferIndex;
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
    rebuildFrameResource(frameResources[bufferIndex]);

    return bufferIndex;
}

void Meshes::insertMesh(std::shared_ptr<Model> mesh, MeshFrameResource& frameResource,
    size_t vertexOffset, size_t indexOffset, size_t meshletOffset,
    size_t meshletVertexOffset, size_t meshletIndexOffset)
{
    auto vertexBufferData = reinterpret_cast<Model::Vertex*>(frameResource.vertexBuffer.GetMappedData());
    auto indexBufferData = reinterpret_cast<Model::Index*>(frameResource.indexBuffer.GetMappedData());
    auto meshletBufferData = reinterpret_cast<MeshletInfo*>(frameResource.meshletBuffer.GetMappedData());
    auto meshletVertexBufferData = reinterpret_cast<uint32_t*>(frameResource.meshletVertexBuffer.GetMappedData());
    auto meshletIndexBufferData = reinterpret_cast<uint8_t*>(frameResource.meshletIndexBuffer.GetMappedData());

    memcpy(&vertexBufferData[vertexOffset], mesh->info.vertices.data(), mesh->info.vertices.size());
    memcpy(&indexBufferData[indexOffset], mesh->info.indices.data(), mesh->info.indices.size());
    for (auto& meshlet : mesh->meshlets)
    {
        meshletBufferData[meshletOffset++] =
            {uint32_t(vertexOffset), uint32_t(indexOffset),
                meshlet.vertexCount, meshlet.indexCount};
        for (auto& meshletVertex : meshlet.vertices)
            meshletVertexBufferData[meshletVertexOffset++] = meshletVertex;
        for (auto& meshletIndex : meshlet.indices)
            meshletIndexBufferData[meshletIndexOffset++] = meshletIndex;
    }
    vertexOffset += mesh->info.vertices.size();
    indexOffset += mesh->info.indices.size();
}
