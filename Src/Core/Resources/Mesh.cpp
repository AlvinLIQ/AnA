#include "Headers/Mesh.hpp"
#include "Headers/Buffer.hpp"
#include "Headers/Device.hpp"
#include "Resources/Headers/ResourceManager.hpp"
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <filesystem>

#ifdef TINYOBJ_LOADER
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#endif

#include "../../3rdParty/meshoptimizer/src/meshoptimizer.h"
#include "../../3rdParty/meshoptimizer/extern/cgltf.h"
#include "../../3rdParty/meshoptimizer/extern/fast_obj.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <unordered_map>

using namespace AnA;

Mesh::Mesh(const MeshData& meshData)
{
    data = std::move(meshData);
    if (data.indices.size()) //indexed model
    {
        center = {};
        for(auto& vertex : data.vertices)
        {
            center += vertex.position;
        }
        center /= float(data.vertices.size());
        radius = 0.0f;
        for(auto& vertex : data.vertices)
        {
            radius = std::max(radius, glm::distance(vertex.position, center));
        }
        buildMeshletsWithOptimizer();
    }
}

Mesh::~Mesh()
{

}

void Mesh::Load(Device* device)
{
    if (loaded)
        return;

    CopyBufferInfo infos[] =
    {
        {&vertexBuffer, data.vertices.data(), data.vertices.size() * sizeof(data.vertices[0])},
        {&meshletVertexBuffer, meshletVertices.data(), meshletVertices.size() * sizeof(uint32_t)},
        {&meshletIndexBuffer, meshletIndices.data(), meshletIndices.size()},
    };
    CopyBuffer(device, numsof(infos), infos);
    loaded = true;
}

void Mesh::Unload()
{
    if (loaded)
    {
        vertexBuffer = {};
        meshletVertexBuffer = {};
        meshletIndexBuffer = {};
        loaded = false;
    }
}

void Mesh::CreateMesh(MeshData& meshData, std::shared_ptr<Mesh>& mesh)
{
    mesh = std::make_shared<Mesh>(std::move(meshData));
}

void Mesh::CreateMeshesFromFile(const char *filePath, std::vector<MeshData>& meshDatas) // split model later
{
    fastObjMesh* mesh = fast_obj_read(filePath);

    //std::set<glm::vec3, Vec3Less> facesSet, edgesSet;

    Mesh::Vertex vertex{};
    uint32_t offset = 0;
    uint32_t triangleIndices[3];
    glm::u8vec3 color{255, 255, 255};
    auto parentPath = std::filesystem::path(filePath).parent_path().string();
    auto resourceManager = Resources::ResourceManager::GetCurrent();
    std::vector<uint32_t> textures(mesh->texture_count);
    for (uint32_t t = 0; t < mesh->texture_count; t++)
        if (mesh->textures[t].path)
        {
            auto path = std::string(mesh->textures[t].path + parentPath.length() + 1);
            resourceManager->AppendTexture(path, &textures[t]);
        }
        else
            textures[t] = 0;

    for (uint32_t o = 0, f, v, fv, i; o < mesh->object_count; o++)
    {
        std::unordered_map<Vertex, Index, VertexHash> vertexMap{};
        MeshData meshData;
        meshData.textureId = 0;
        meshData.minBounding = glm::vec3(FLT_MAX);
        meshData.maxBounding = glm::vec3(-FLT_MAX);
        auto& obj = mesh->objects[o];
        offset = obj.index_offset;
        for (f = 0; f < obj.face_count; f++)
        {
            fv = mesh->face_vertices[f + obj.face_offset];
            triangleIndices[0] = offset;
            if (mesh->material_count)
            {
                auto& material = mesh->materials[mesh->face_materials[f + obj.face_offset]];
                color = {
                    uint8_t(material.Kd[0] * 255.0f),
                    uint8_t(material.Kd[1] * 255.0f),
                    uint8_t(material.Kd[2] * 255.0f),
                };
                if (material.map_Kd < mesh->texcoord_count)
                    meshData.textureId = textures[material.map_Kd];
            }
            for (v = 0; v + 1 < fv; v++) // deal with face triangulation
            {
                triangleIndices[1] = offset + v;
                triangleIndices[2] = offset + v + 1;
                for (i = 0; i < 3; i++)
                {
                    auto& index = mesh->indices[triangleIndices[i]];
                    vertex.position = *reinterpret_cast<glm::vec3*>(&mesh->positions[3 * index.p]);
                    ExtractPitchYaw(*reinterpret_cast<glm::vec3*>(&mesh->normals[3 * index.n]), vertex.pitch, vertex.yaw);
                    vertex.color = color;
                    vertex.uv = *reinterpret_cast<glm::vec2*>(&mesh->texcoords[2 * index.t]);
                    vertex.textureId = meshData.textureId;
                    auto result = vertexMap.find(vertex);
                    if (result != vertexMap.end())
                    {
                        meshData.indices.push_back(result->second);
                        //meshData.nodes.back().indices.push_back(result->second);
                    }
                    else
                    {
                        meshData.indices.push_back(static_cast<Index>(vertexMap.size()));
                        //meshData.nodes.back().indices.push_back(static_cast<Index>(vertexMap.size()));

                        vertexMap.insert(std::pair<Vertex, Index>(vertex, static_cast<Index>(vertexMap.size())));
                        meshData.vertices.push_back(vertex);
                        meshData.minBounding = glm::min(meshData.minBounding, vertex.position);
                        meshData.maxBounding = glm::max(meshData.maxBounding, vertex.position);
                    }
                }
            }
            offset += fv;
        }
        meshDatas.emplace_back(meshData);
    }
    fast_obj_destroy(mesh);
}

void Mesh::CreateVerticesFromFile(const char *filePath, std::vector<Vertex> &vertices)
{
    auto data = ReadFile(filePath);
    auto str = reinterpret_cast<char*>(data.data());
    for (size_t j = 0; j < data.size();)
    {
        Mesh::Vertex vertex{};
        sscanf(str + j, "%f,%f,%f\n",
            &vertex.position.x, &vertex.position.y, &vertex.position.z);

        vertices.push_back(vertex);
        while(str[j++] != '\n');
    }
}

float Slope(const glm::vec3& v)
{
    float h = sqrtf(v.x * v.x + v.y * v.y);
    return std::abs(v.z / h);
}

bool Mesh::CreateQuad(std::vector<Vertex> &vertices, std::vector<Index> &indices, Index a, Index b, Index c, Index d)
{
    auto& va = vertices[a].position;
    auto& vb = vertices[b].position;
    auto& vc = vertices[c].position;
    auto& vd = vertices[d].position;

    glm::vec3 d1 = (vb - va);
    glm::vec3 d2 = (vc - va);
    glm::vec3 d3 = (vd - vb);
    glm::vec3 d4 = (vc - vb);

    float l = std::max(std::abs(glm::dot(glm::normalize(d1), glm::normalize(d2))), std::abs(glm::dot(glm::normalize(d3), glm::normalize(d4))));
    if (d % 256 < 255)
    {
        l = std::max(l, std::abs(glm::dot(glm::normalize(vertices[d + 1].position - vb), glm::normalize(vertices[c + 1].position - vb))));
        l = std::max(l, std::abs(glm::dot(glm::normalize(vertices[d + 1].position - va), glm::normalize(vertices[c + 1].position - va))));
    }
    float sl = glm::length(d1) + glm::length(d2) + glm::length(d3) + glm::length(d4);
    if (sl > 0.4f)
    {
        return false;
    }

    indices.push_back(a);
    indices.push_back(b);
    indices.push_back(c);

    indices.push_back(b);
    indices.push_back(d);
    indices.push_back(c);
    return true;
}

void Mesh::ExtractPitchYaw(glm::vec3& normal, uint16_t& pitch, uint16_t& yaw)
{
    yaw = uint16_t((glm::degrees(atan2(normal.x, normal.z)) / 360.0f) * 65535.0f);
    pitch = uint16_t((glm::degrees(asin(normal.y)) / 360.0f) * 65535.0f);
}

void Mesh::buildMeshletsWithOptimizer()
{
    const uint32_t maxVerticesPerMeshlet = numsof(MeshletData::vertices);
    const uint32_t maxIndicesPerMeshlet = numsof(MeshletData::indices);

    meshlets.clear();

    auto& meshIndices = data.indices;
    // Estimate output sizes
    size_t maxMeshlets = meshopt_buildMeshletsBound(uint32_t(data.indices.size()), maxVerticesPerMeshlet, maxIndicesPerMeshlet / 3);
    std::vector<meshopt_Meshlet> meshopt_meshlets(maxMeshlets);
    meshletVertices.resize(maxMeshlets * maxVerticesPerMeshlet);
    meshletIndices.resize(maxMeshlets * maxIndicesPerMeshlet);
    size_t actualMeshletCount = meshopt_buildMeshlets(
        meshopt_meshlets.data(),
        meshletVertices.data(),
        meshletIndices.data(),
        meshIndices.data(),
        meshIndices.size(),
        &data.vertices.data()->position.x, // Optional vertex data pointer, can be nullptr
        uint32_t(data.vertices.size()),
        sizeof(Mesh::Vertex),
        maxVerticesPerMeshlet,
        maxIndicesPerMeshlet / 3,
        0.6f
    );
    meshopt_meshlets.resize(actualMeshletCount);
    auto& last = meshopt_meshlets.back();
    meshletVertices.resize(last.vertex_offset + last.vertex_count);
    meshletIndices.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3u));

    Meshlet meshlet{};
    for (auto& meshletInfo : meshopt_meshlets)
    {
        meshlet.vertexOffset = meshletInfo.vertex_offset;
        meshlet.indexOffset = meshletInfo.triangle_offset;
        meshlet.indexCount = static_cast<uint32_t>(meshletInfo.triangle_count) * 3;
        meshlet.vertexCount = static_cast<uint32_t>(meshletInfo.vertex_count);
        glm::vec3 minBounding{std::numeric_limits<float>::max()};
        glm::vec3 maxBounding{std::numeric_limits<float>::min()};
        for (uint32_t i = 0; i < meshlet.vertexCount; i++)
        {
            auto& vertex = data.vertices[meshletVertices[i + meshletInfo.vertex_offset]];
            minBounding = glm::min(minBounding, vertex.position);
            maxBounding = glm::max(maxBounding, vertex.position);
        }
        //meshopt_optimizeMeshlet(meshlet.vertices, meshlet.indices, meshletInfo.triangle_count, meshletInfo.vertex_count);
        meshopt_Bounds bounds = meshopt_computeMeshletBounds(&meshletVertices[meshletInfo.vertex_offset],
            &meshletIndices[meshletInfo.triangle_offset], meshletInfo.triangle_count,
                &data.vertices.data()->position.x,
                uint32_t(data.vertices.size()), sizeof(Mesh::Vertex));
        meshlet.bounding.normal = *reinterpret_cast<glm::vec3*>(&bounds.cone_axis);
        float len = glm::length(meshlet.bounding.normal);
        if (len != 0.)
            meshlet.bounding.normal /= len;
        else
            meshlet.bounding.normal = glm::vec3(1., 0., 0.);
        meshlet.bounding.coneApex = *reinterpret_cast<glm::vec3*>(&bounds.cone_apex);
        //meshlet.center = {bounds.center[0], bounds.center[1], bounds.center[2]};
        meshlet.bounding.center = (minBounding + maxBounding) * 0.5f;
        meshlet.bounding.cutoff = bounds.cone_cutoff;
        //meshlet.radius = bounds.radius;

        float maxDistance = 0.0f;
        for (uint32_t i = 0; i < meshlet.vertexCount; i++)
        {
            float distance = glm::distance(meshlet.bounding.center, data.vertices[meshletVertices[i + meshletInfo.vertex_offset]].position);
            if (distance > maxDistance)
            {
                maxDistance = distance;
                meshlet.bounding.radius = maxDistance;
            }
        }
        meshlets.push_back(meshlet);
    }
}
