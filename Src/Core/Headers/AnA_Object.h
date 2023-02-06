#pragma once

#include "AnA_Model.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <stb_image.h>

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
            auto transform = glm::translate(glm::mat4{1.f}, translation);

            transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
            transform = glm::rotate(transform, rotation.x, {1.f, 0.f, 0.f});
            transform = glm::rotate(transform, rotation.z, {0.f, 1.f, 1.f});

            transform = glm::scale(transform, scale);
            return transform;
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