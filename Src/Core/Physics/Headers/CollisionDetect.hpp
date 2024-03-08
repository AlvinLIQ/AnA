#pragma once

#include "../../Headers/Object.hpp"
#include <glm/glm.hpp>

namespace AnA
{
    namespace Physics
    {
        static bool IsCollided(Object& object1, Object& object2);
        static bool IsCollided(const glm::vec2* projections, const glm::mat3* transforms, uint32_t projectionCount, float& projectionScale, const glm::vec3& projectionTranslation, const Model::Vertex* vertices, const uint32_t vertexCount, const glm::vec3& vertexTranslation, const float& vertexScale);
        static bool IsCollided2D();
    }
}