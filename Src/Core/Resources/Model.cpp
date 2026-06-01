#include "Headers/Model.hpp"
#include "Headers/Device.hpp"
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

Model::Model(const ModelInfo& modelInfo)
{
    info = std::move(modelInfo);
    if (modelInfo.indices.size()) //indexed model
    {
        center = {};
        for(auto& vertex : info.vertices)
        {
            center += vertex.position;
        }
        center /= float(info.vertices.size());
        radius = 0.0f;
        for(auto& vertex : info.vertices)
        {
            radius = std::max(radius, glm::distance(vertex.position, center));
        }
        buildMeshletsWithOptimizer();
    }
}

Model::~Model()
{

}

void Model::CreateModelFromFile(const char *filePath, std::shared_ptr<Model>& model)
{
    ModelInfo modelInfo{};
    CreateMeshFromFile(filePath, modelInfo);

    modelInfo.indexStep = static_cast<Index>(modelInfo.vertices.size());
    model = std::make_shared<Model>(modelInfo);
    model->Path = filePath;
}
#ifdef TINYOBJ_LOADER
void Model::CreateMeshFromFile(const char *filePath, ModelInfo& modelInfo)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib,& shapes,& materials,& warn,& err, filePath, "Models/"))
        throw std::runtime_error(warn + err);

    std::unordered_map<Vertex, Index, VertexHash> vertexMap;
    //std::set<glm::vec3, Vec3Less> facesSet, edgesSet;
    modelInfo.minBounding = glm::vec3(FLT_MAX);
    modelInfo.maxBounding = glm::vec3(-FLT_MAX);
    for (const auto& shape : shapes)
    {
        modelInfo.nodes.push_back({});
        for (size_t i = 0; i < shape.mesh.indices.size(); i++)
        {
            const auto& index = shape.mesh.indices[i];
            Vertex vertex{};

            if (index.vertex_index >= 0)
            {
                vertex.position =
                {
                    attrib.vertices[3 * static_cast<size_t>(index.vertex_index)],
                    attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 1],
                    attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 2]
                };
                /*
                auto colorIndex = 3 * index.vertex_index + 2;
                if (colorIndex < attrib.colors.size())
                {
                    vertex.color =
                    {
                        attrib.colors[colorIndex - 2],
                        attrib.colors[colorIndex - 1],
                        attrib.colors[colorIndex],
                    };
                }
                if (materials.size())
                {
                    vertex.color =
                    {
                        materials[0].diffuse[0],
                        materials[0].diffuse[1],
                        materials[0].diffuse[2]
                    };
                }*/
            }
            if (index.normal_index >= 0)
            {
                vertex.normal =
                {
                    attrib.normals[3 * static_cast<size_t>(index.normal_index)],
                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 1],
                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 2]
                };
            }
            if (index.texcoord_index >= 0)
            {
                vertex.uv =
                {
                    attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index)],
                    attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 1],
                };
            }
            auto result = vertexMap.find(vertex);
            if (result != vertexMap.end())
            {
                modelInfo.indices.push_back(result->second);
                modelInfo.nodes.back().indices.push_back(result->second);
            }
            else
            {
                modelInfo.indices.push_back(static_cast<Index>(vertexMap.size()));
                modelInfo.nodes.back().indices.push_back(static_cast<Index>(vertexMap.size()));

                vertexMap.insert(std::pair<Vertex, Index>(vertex, static_cast<Index>(vertexMap.size())));
                modelInfo.vertices.push_back(vertex);
                modelInfo.minBounding = glm::min(modelInfo.minBounding, vertex.position);
                modelInfo.maxBounding = glm::max(modelInfo.maxBounding, vertex.position);
            }
/*
            if (i > 0 && (i + 1) % 3 == 0)
            {
                Index v0 = modelInfo.indices[i - 2], v1 = modelInfo.indices[i - 1], v2 = modelInfo.indices[i];
                glm::vec3 edges[3] = {
                glm::normalize(modelInfo.vertices[v2].position - modelInfo.vertices[v1].position),
                glm::normalize(modelInfo.vertices[v2].position - modelInfo.vertices[v0].position),
                glm::normalize(modelInfo.vertices[v1].position - modelInfo.vertices[v0].position)
                };
                for (auto& edge : edges)
                {
                    if (edge.x < 0 ||
                        (edge.x == 0 && edge.y < 0) ||
                        (edge.x == 0 && edge.y == 0 && edge.z < 0))
                        edge = -edge;
                    if (edgesSet.emplace(edge).second)
                        modelInfo.edges.push_back(edge);
                }

                glm::vec3 normal =
                    glm::normalize(
                        glm::cross(modelInfo.vertices[v1].position -
                            modelInfo.vertices[v0].position,
                            modelInfo.vertices[v2].position - modelInfo.vertices[v0].position));
                if (facesSet.emplace(normal).second)
                    modelInfo.faces.push_back(normal);
            }*/
        }
    }
}
#else
void Model::CreateMeshFromFile(const char *filePath, ModelInfo& modelInfo) // split model later
{
    fastObjMesh* mesh = fast_obj_read(filePath);

    std::unordered_map<Vertex, Index, VertexHash> vertexMap{};
    //std::set<glm::vec3, Vec3Less> facesSet, edgesSet;
    modelInfo.minBounding = glm::vec3(FLT_MAX);
    modelInfo.maxBounding = glm::vec3(-FLT_MAX);
    Model::Vertex vertex{};
    uint32_t offset = 0;
    uint32_t triangleIndices[3];
    glm::u8vec3 color{255, 255, 255};
    auto parentPath = std::filesystem::path(filePath).parent_path().string();
    for (uint t = 0; t < mesh->texture_count; t++)
        if (mesh->textures[t].path)
            printf("%s\n", mesh->textures[t].path + parentPath.length() + 1);
    for (uint32_t f = 0, v, fv, i; f < mesh->face_count; f++)
    {
        fv = mesh->face_vertices[f];
        triangleIndices[0] = offset;
        if (mesh->material_count)
        {
            auto& material = mesh->materials[mesh->face_materials[f]];
            color = {
                uint8_t(material.Kd[0] * 255.0f),
                uint8_t(material.Kd[1] * 255.0f),
                uint8_t(material.Kd[2] * 255.0f),
            };
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
                auto result = vertexMap.find(vertex);
                if (result != vertexMap.end())
                {
                    modelInfo.indices.push_back(result->second);
                    //modelInfo.nodes.back().indices.push_back(result->second);
                }
                else
                {
                    modelInfo.indices.push_back(static_cast<Index>(vertexMap.size()));
                    //modelInfo.nodes.back().indices.push_back(static_cast<Index>(vertexMap.size()));

                    vertexMap.insert(std::pair<Vertex, Index>(vertex, static_cast<Index>(vertexMap.size())));
                    modelInfo.vertices.push_back(vertex);
                    modelInfo.minBounding = glm::min(modelInfo.minBounding, vertex.position);
                    modelInfo.maxBounding = glm::max(modelInfo.maxBounding, vertex.position);
                }
            }
        }
        offset += fv;

    }
    fast_obj_destroy(mesh);
}
#endif
void Model::CreateVerticesFromFile(const char *filePath, std::vector<Vertex> &vertices)
{
    auto data = ReadFile(filePath);
    auto str = reinterpret_cast<char*>(data.data());
    for (size_t j = 0; j < data.size();)
    {
        Model::Vertex vertex{};
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

bool Model::CreateQuad(std::vector<Vertex> &vertices, std::vector<Index> &indices, Index a, Index b, Index c, Index d)
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

void Model::CreateTerrainFromVertices(std::vector<Vertex> &vertices, std::vector<Index> &indices, size_t period)
{
    assert(period > 1);
    assert(vertices.size() % period == 0);
    size_t rowCount = vertices.size() / period;
    assert(rowCount > 1);
    //offset = i, -period 0 1, -period, -period + 1, 1
    Index a, b, c, d;
    for (size_t i = period, vi; i < vertices.size(); i += period)
    {
        a = Index(i);
        b = Index(i - period);
        for (vi = 0; vi < period - 1; vi++)
        {
            c = Index(vi + i + 1);
            d = Index(vi + i - period + 1);

            Index t = c, u = d;

            while (t % period < period - 1 && u % period < period - 1 && !CreateQuad(vertices, indices, a, b, t, u))
            {
                ++t;
                ++u;
            }
            a = c;
            b = d;
        }
    }
}

void Model::ExtractPitchYaw(glm::vec3& normal, uint16_t& pitch, uint16_t& yaw)
{
    yaw = uint16_t((glm::degrees(atan2(normal.x, normal.z)) / 360.0f) * 65535.0f);
    pitch = uint16_t((glm::degrees(asin(normal.y)) / 360.0f) * 65535.0f);
}

void Model::buildMeshlets()
{
    constexpr uint32_t maxVerticesPerMeshlet = numsof(Meshlet::vertices);
    constexpr uint32_t maxIndicesPerMeshlet = numsof(Meshlet::indices);
    meshlets.clear();

    uint32_t totalIndices = uint32_t(info.indices.size());
    uint32_t indexOffset = 0;
    uint32_t indexEnd;
    while (indexOffset < totalIndices)
    {
        std::unordered_map<uint32_t, uint8_t> vertexMap{};
        Meshlet meshlet{};
        indexEnd = std::min(indexOffset + maxIndicesPerMeshlet, totalIndices);
        for (; indexOffset < indexEnd; indexOffset+= 3)
        {
            uint32_t iid = indexOffset;
            if (vertexMap.try_emplace(info.indices[iid], static_cast<uint8_t>(vertexMap.size())).second)
            {
                if (meshlet.vertexCount >= maxVerticesPerMeshlet)
                    break;
                meshlet.vertices[meshlet.vertexCount++] = info.indices[iid];
            }
            if (vertexMap.try_emplace(info.indices[iid + 1], static_cast<uint8_t>(vertexMap.size())).second)
            {
                if (meshlet.vertexCount >= maxVerticesPerMeshlet)
                    break;
                meshlet.vertices[meshlet.vertexCount++] = info.indices[iid + 1];
            }
            if (vertexMap.try_emplace(info.indices[iid + 2], static_cast<uint8_t>(vertexMap.size())).second)
            {
                if (meshlet.vertexCount > maxVerticesPerMeshlet)
                    break;
                meshlet.vertices[meshlet.vertexCount++] = info.indices[iid + 2];
            }
            meshlet.indices[meshlet.indexCount++] = vertexMap[info.indices[iid + 0]];
            meshlet.indices[meshlet.indexCount++] = vertexMap[info.indices[iid + 1]];
            meshlet.indices[meshlet.indexCount++] = vertexMap[info.indices[iid + 2]];
        }
        if (indexOffset < indexEnd)
            indexOffset -= 3;
        meshletIndexCount += meshlet.indexCount;
        meshletVertexCount += meshlet.vertexCount;
        meshlets.push_back(meshlet);
    }
}

void Model::buildMeshletsWithOptimizer()
{
    const uint32_t maxVerticesPerMeshlet = numsof(Meshlet::vertices);
    const uint32_t maxIndicesPerMeshlet = numsof(Meshlet::indices);

    meshlets.clear();

    auto& meshIndices = info.indices;
    // Estimate output sizes
    size_t maxMeshlets = meshopt_buildMeshletsBound(uint32_t(info.indices.size()), maxVerticesPerMeshlet, maxIndicesPerMeshlet / 3);
    std::vector<meshopt_Meshlet> meshopt_meshlets(maxMeshlets);
    std::vector<uint32_t> uniqueVertexIndices(maxMeshlets * maxVerticesPerMeshlet);
    std::vector<uint8_t> primitiveIndices(maxMeshlets * maxIndicesPerMeshlet);
    size_t actualMeshletCount = meshopt_buildMeshlets(
        meshopt_meshlets.data(),
        uniqueVertexIndices.data(),
        primitiveIndices.data(),
        meshIndices.data(),
        meshIndices.size(),
        &info.vertices.data()->position.x, // Optional vertex data pointer, can be nullptr
        uint32_t(info.vertices.size()),
        sizeof(Model::Vertex),
        maxVerticesPerMeshlet,
        maxIndicesPerMeshlet / 3,
        0.6f
    );
    meshopt_meshlets.resize(actualMeshletCount);
    auto& last = meshopt_meshlets.back();
    uniqueVertexIndices.resize(last.vertex_offset + last.vertex_count);
    primitiveIndices.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3u));

    for (auto& meshletInfo : meshopt_meshlets)
    {
        Meshlet meshlet{};
        meshlet.indexCount = static_cast<uint32_t>(meshletInfo.triangle_count) * 3;
        meshlet.vertexCount = static_cast<uint32_t>(meshletInfo.vertex_count);
        meshletIndexCount += meshlet.indexCount;
        meshletVertexCount += meshlet.vertexCount;
        for (uint32_t i = 0; i < meshlet.indexCount; i++)
        {
            meshlet.indices[i] = primitiveIndices[i + meshletInfo.triangle_offset];
        }
        glm::vec3 minBounding{std::numeric_limits<float>::max()};
        glm::vec3 maxBounding{std::numeric_limits<float>::min()};
        for (uint32_t i = 0; i < meshlet.vertexCount; i++)
        {
            meshlet.vertices[i] = uniqueVertexIndices[i + meshletInfo.vertex_offset];
            auto& vertex = info.vertices[meshlet.vertices[i]];
            minBounding = glm::min(minBounding, vertex.position);
            maxBounding = glm::max(maxBounding, vertex.position);
        }
        //meshopt_optimizeMeshlet(meshlet.vertices, meshlet.indices, meshletInfo.triangle_count, meshletInfo.vertex_count);
        meshopt_Bounds bounds = meshopt_computeMeshletBounds(&uniqueVertexIndices[meshletInfo.vertex_offset],
            &primitiveIndices[meshletInfo.triangle_offset], meshletInfo.triangle_count,
                &info.vertices.data()->position.x,
                uint32_t(info.vertices.size()), sizeof(Model::Vertex));
        meshlet.normal = *reinterpret_cast<glm::vec3*>(&bounds.cone_axis);
        float len = glm::length(meshlet.normal);
        if (len != 0.)
            meshlet.normal /= len;
        else
            meshlet.normal = glm::vec3(1., 0., 0.);
        meshlet.coneApex = *reinterpret_cast<glm::vec3*>(&bounds.cone_apex);
        //meshlet.center = {bounds.center[0], bounds.center[1], bounds.center[2]};
        meshlet.center = (minBounding + maxBounding) * 0.5f;
        meshlet.center = meshlet.center;
        meshlet.cutoff = bounds.cone_cutoff;
        //meshlet.radius = bounds.radius;

        float maxDistance = 0.0f;
        for (uint32_t i = 0; i < meshlet.vertexCount; i++)
        {
            float distance = glm::distance(meshlet.center, info.vertices[uniqueVertexIndices[i + meshletInfo.vertex_offset]].position);
            if (distance > maxDistance)
            {
                maxDistance = distance;
                meshlet.radius = maxDistance;
            }
        }
        meshlets.push_back(meshlet);
    }
}
