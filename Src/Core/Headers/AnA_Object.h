#pragma once

#include "AnA_Model.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
namespace AnA
{
    struct Transform2DComponent
    {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2()
        {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotMatrix{{c, s}, {-s, c}};
            glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};
            return rotMatrix * scaleMat;
        }
    };
    class AnA_Object
    {
    public:
        using id_t = unsigned int;

        AnA_Object();
        ~AnA_Object();

        id_t GetId()
        {
            return id;
        }

        AnA_Model* Model;
        glm::vec3 Color{};
        Transform2DComponent Transform2D{};

    private:
        id_t id;
    };
}