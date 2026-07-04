#pragma once

#include <glm/glm.hpp>
#include "../../Resources/Headers/Mesh.hpp"

namespace AnA
{
    namespace Physics
    {
        inline bool IsCollided(const glm::vec2* projections, const glm::mat3* transforms, uint32_t projectionCount, float& projectionScale, const glm::vec3& projectionTranslation, const Mesh::Vertex* vertices, const uint32_t vertexCount, const glm::vec3& vertexTranslation, const float& vertexScale);
        inline bool IsCollided2D();
    }
}
