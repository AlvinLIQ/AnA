#include "Headers/Model.hpp"
#include "Headers/Device.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <memory>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../3rdParty/tinyobjloader/tiny_obj_loader.h"

#include "../../3rdParty/meshoptimizer/src/meshoptimizer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <unordered_map>


using namespace AnA;

Model::Model(const ModelInfo& modelInfo)
{
    info = std::move(modelInfo);
    buildMeshletsWithOptimizer();
}

Model::~Model()
{

}

void Model::CreateModelFromFile(const char *filePath, std::shared_ptr<Model>& model)
{
    ModelInfo modelInfo{};
    CreateMeshFromFile(filePath, modelInfo.vertices, modelInfo.indices);
/*
    const glm::vec<2, int> sets[] = {{0, 1}, {0, 2}, {1, 2}};
    for (size_t i = 0, j, k = 0; i < modelInfo.indices.size(); i += k)
    {
        for (k = 0; k < 3; k++)
        {
            glm::vec3 xBase = glm::normalize(modelInfo.vertices[modelInfo.indices[i + static_cast<size_t>(sets[k].y)]].position - modelInfo.vertices[modelInfo.indices[i + static_cast<size_t>(sets[k].x)]].position);
            glm::vec3 yBase = glm::mat3({0.0, -1.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}) * xBase;
            glm::vec3 zBase = glm::mat3({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}) * xBase;
            glm::mat3 transform{xBase, yBase, zBase};
            glm::vec2 currentProjection{(transform * modelInfo.vertices[modelInfo.indices[0]].position).y};
            //1.0
            //glm::length(currentPlane);
            for (j = 1; j < modelInfo.indices.size(); j++)
            {
                auto positionY = (transform * modelInfo.vertices[modelInfo.indices[j]].position).y;
                if (positionY < currentProjection.x)
                {
                    currentProjection[0] = positionY;
                }
                else if (positionY > currentProjection.y)
                {
                    currentProjection[1] = positionY;
                }
            }
            modelInfo.vertexProjections.push_back(currentProjection);
        }
    }*/
    modelInfo.indexStep = static_cast<Index>(modelInfo.vertices.size());
    model = std::make_shared<Model>(modelInfo);
    model->Path = filePath;
}

void Model::CreateMeshFromFile(const char *filePath, std::vector<Vertex>& vertices, std::vector<Index>& indices, size_t vertexOffset)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib,& shapes,& materials,& warn,& err, filePath, "Models/"))
        throw std::runtime_error(warn + err);

    std::unordered_map<Vertex, Index, VertexHash> vertexMap;
    for (const auto& shape : shapes)
    {
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
                indices.push_back(result->second + static_cast<uint32_t>(vertexOffset));
            }
            else
            {
                indices.push_back(static_cast<Index>(vertexMap.size() + vertexOffset));
                vertexMap.insert(std::pair<Vertex, Index>(vertex, vertexMap.size()));
                vertices.push_back(vertex);
            }
        }
    }
}

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

void CalculateNormal(std::vector<Model::Vertex>& vertices, size_t index, size_t period)
{
    size_t imp = index % period;
    auto& left = imp ? vertices[index - 1] : vertices[index];
    auto& right = imp != period - 1 ?  vertices[index + 1] : vertices[index];
    auto& up = index >= period ? vertices[index - period] : vertices[index];
    auto& down = index + period < vertices.size() ? vertices[index + period] : vertices[index];
    auto v1 = right.position - left.position;
    auto v2 = down.position - up.position;
    vertices[index].normal = glm::normalize(glm::cross(v1, v2));
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
        a = i;
        b = i - period;
        for (vi = 0; vi < period - 1; vi++)
        {
            c = vi + i + 1;
            d = vi + i - period + 1;

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
        0.5f
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
        float maxDistance = 0.0f;
        for (uint32_t i = 0; i < meshlet.vertexCount; i++)
        {
            float distance = glm::distance(meshlet.center, info.vertices[uniqueVertexIndices[i + meshletInfo.vertex_offset]].position);
            if (distance > maxDistance)
            {
                maxDistance = distance;
                meshlet.farVertexID = i;
            }
        }
        meshlets.push_back(meshlet);
    }
}