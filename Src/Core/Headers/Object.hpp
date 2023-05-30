#pragma once

#include "Model.hpp"
#include "Texture.hpp"
#include "Types.hpp"
#include <memory>
#include <vector>

#define SHAPE_TYPE uint32_t
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
    };

    class Object
    {
    public:
        using id_t = unsigned int;

        Object();
        virtual ~Object();

        id_t GetId()
        {
            return id;
        }

        std::shared_ptr<Model> Model;
        glm::vec3 Color{};
        ItemProperties Properties;

        Texture* GetTexture();
        void SetTexture(std::shared_ptr<Texture> newTexture);

        VkDescriptorSet& GetDescriptorSet(int index)
        {
            return descriptorSets[index];
        }
        
        static void CreateShape(SHAPE_TYPE sType,glm::vec2 offset, glm::vec2 size, std::optional<glm::vec3> color, ItemProperties* pRectangleProperties)
        {
            pRectangleProperties->sType = sType;
            pRectangleProperties->transform.translation = {offset , 0.f};
            pRectangleProperties->transform.scale = {size, 1.f};
            if (color.has_value())
                pRectangleProperties->color = color;
        }

        virtual void PrepareDraw();
    private:
        id_t id;
        std::shared_ptr<Texture> texture;
        std::vector<VkDescriptorSet> descriptorSets;
    };
}