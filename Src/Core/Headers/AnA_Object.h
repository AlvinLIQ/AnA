#pragma once

#include "AnA_Model.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>

#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_CIRCLE 2
#define ANA_CURVED_RECTANGLE 3
#define ANA_MODEL 4

namespace AnA
{
    struct ShapePushConstantData
    {
        glm::mat2 transform {1.f};
        glm::uint32_t sType;
        alignas(8) glm::vec2 offset;
        glm::vec2 resolution;
        alignas(16) glm::vec3 color;
    };
    struct Transform2D
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
    struct ItemProperties
    {
        Transform2D transform;
        uint32_t sType{ANA_RECTANGLE};
        std::optional<glm::vec3> color;
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
        std::vector<ItemProperties> ItemsProperties;

    private:
        id_t id;
    };
}