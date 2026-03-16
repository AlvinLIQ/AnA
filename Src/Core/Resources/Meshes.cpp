#include "Headers/Meshes.hpp"
#include "Resources/Headers/Model.hpp"

using namespace AnA;
using namespace Resources;

uint32_t meshId = 0;

Meshes::Meshes()
{
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

    auto mesh = MeshMap[id];

    loadedSet.emplace(id);
}

void Meshes::prepareFrameResources()
{
}
