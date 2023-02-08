#pragma once

#include "AnA_Model.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#define ANA_SHAPE_TYPE uint32_t
#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_ELLIPSE 2
#define ANA_CURVED_RECTANGLE 3
#define ANA_MODEL 4

namespace AnA
{
    struct ShapePushConstantData
    {
        glm::mat4 transform {1.f};
        glm::uint32_t sType;
        alignas(8) glm::vec2 resolution;
        alignas(16) glm::vec3 color;
    };
    struct Transform
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation;

        glm::mat4 mat4()
        {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4
            {
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}
            };
        }
    };
    struct ItemProperties
    {
        Transform transform;
        uint32_t sType{ANA_RECTANGLE};
        std::optional<VkImage> texture;
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

        static void CreateShape(ANA_SHAPE_TYPE sType,glm::vec2 offset, glm::vec2 size, std::optional<glm::vec3> color, ItemProperties* pRectangleProperties)
        {
            pRectangleProperties->sType = sType;
            pRectangleProperties->transform.translation = {offset , 0.f};
            pRectangleProperties->transform.scale = {size, 1.f};
        }

    private:
        id_t id;
    };
}