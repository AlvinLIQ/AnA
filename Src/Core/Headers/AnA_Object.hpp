#pragma once

#include "AnA_Model.hpp"
#include "AnA_Types.hpp"
#include <vector>

#define ANA_SHAPE_TYPE uint32_t
#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_ELLIPSE 2
#define ANA_CURVED_RECTANGLE 3
#define ANA_MODEL 4

namespace AnA
{
    struct ObjectPushConstantData
    {
//        glm::mat4 projectionMatrix {1.f};
        glm::mat4 transformMatrix {1.f};
        glm::uint32_t sType;
        alignas(8) glm::vec2 resolution;
        alignas(16) glm::vec3 color;
    };

    struct ItemProperties
    {
        Transform transform;
        uint32_t sType{ANA_RECTANGLE};
        std::optional<glm::vec3> color;
        std::optional<VkImage> texture;
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
            if (color.has_value())
                pRectangleProperties->color = color;
        }

    private:
        id_t id;
    };
}