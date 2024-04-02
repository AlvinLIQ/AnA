#pragma once

#include <glm/glm.hpp>
#include "Model.hpp"
#include "../../Headers/Types.hpp"

namespace AnA
{
    class Mesh
    {
    public:
        Mesh(std::string filePath);
        ~Mesh();

        AnA::Transform Transform{};

        std::vector<Model::Vertex> Vertices{};
        std::vector<Model::Index> Indices{};
    };
}