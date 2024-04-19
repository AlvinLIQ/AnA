#pragma once

#include <glm/glm.hpp>
#include "../../Resources/Headers/Model.hpp"

namespace AnA
{
    namespace Physics
    {
        static bool IsCollided(const glm::vec2* projections, const glm::mat3* transforms, uint32_t projectionCount, float& projectionScale, const glm::vec3& projectionTranslation, const Model::Vertex* vertices, const uint32_t vertexCount, const glm::vec3& vertexTranslation, const float& vertexScale);
        static bool IsCollided2D();
    }
}