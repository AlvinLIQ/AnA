#include "Headers/CollisionDetect.hpp"

using namespace AnA;
using namespace Physics;

bool AnA::Physics::IsCollided(const glm::vec2* projections, const glm::mat3* transforms, uint32_t projectionCount, float& projectionScale, const glm::vec3& projectionTranslation, const Mesh::Vertex* vertices, const uint32_t vertexCount, const glm::vec3& vertexTranslation, const float& vertexScale)
{
    for (uint32_t i = 0; i < projectionCount; i++)
    {
        auto& transform = transforms[i];
        auto proj = projections[i] * projectionScale + glm::vec2((transform * projectionTranslation).y);
        for (uint32_t j = 0; j < vertexCount; j++)
        {
            auto vertexPosition = transform * (vertices[j].position * vertexScale + vertexTranslation);
            if (proj.x > vertexPosition.y || proj.y < vertexPosition.y)
            {
                return false;
            }
        }
    }
    return true;
}
bool AnA::Physics::IsCollided2D()
{

    return true;
}
