#include "Headers/Model.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <memory>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../3rdParty/tinyobjloader/tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <unordered_map>


using namespace AnA;

Model::Model(const ModelInfo& modelInfo)
{
    info = std::move(modelInfo);
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
