#include "Headers/CollisionDetect.hpp"

using namespace AnA;
using namespace Physics;

bool AnA::Physics::IsCollided(Object& object1, Object& object2)
{
    return IsCollided(object1.Model->GetVertexProjections().data(), object1.Model->GetTransforms().data(), 
                            object1.Model->GetVertexProjections().size(), object1.Properties.transform.scale.y, 
                            object1.Properties.transform.translation, object2.Model->GetVertices().data(), object2.Model->GetVertices().size(),
                            object2.Properties.transform.translation, object2.Properties.transform.scale.y);
}

bool AnA::Physics::IsCollided(const glm::vec2* projections, const glm::mat3* transforms, uint32_t projectionCount, float& projectionScale, const glm::vec3& projectionTranslation, const Model::Vertex* vertices, const uint32_t vertexCount, const glm::vec3& vertexTranslation, const float& vertexScale)
{
    for (int i = 0; i < projectionCount; i++)
    {
        auto& transform = transforms[i];
        auto proj = projections[i] * projectionScale + glm::vec2((transform * projectionTranslation).y);
        for (int j = 0; j < vertexCount; j++)
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