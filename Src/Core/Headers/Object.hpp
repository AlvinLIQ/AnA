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
#define ANA_TEXT 5

namespace AnA
{
    struct ObjectPushConstantData
    {
//        glm::mat4 projectionMatrix {1.f};
        glm::mat4 transformMatrix {1.f};
        glm::uint32_t sType;
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
        VkDescriptorSet& GetPropertiesDescriptorSet()
        {
            return descriptorSet;
        }

        std::unique_ptr<Texture> Texture;
        
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
        VkDescriptorPool descriptorPool;
        void createDescriptorPool();
        VkDescriptorSet descriptorSet;
        void createDescriptorSet();
    };

    typedef char ANA_OBJECTS_UPDATE_FLAG_BIT;
    static const ANA_OBJECTS_UPDATE_FLAG_BIT ANA_OBJECTS_UPDATE_COMMAND_BUFFER = 1;
    static const ANA_OBJECTS_UPDATE_FLAG_BIT ANA_OBJECTS_UPDATE_UNIFORM_BUFFER = 2;
    static const ANA_OBJECTS_UPDATE_FLAG_BIT ANA_OBJECTS_UPDATE_ALL = 3;

    class Objects
    {
    public:
        void Destroy()
        {
            for (auto& object : objects)
                delete object;
        }
        const std::vector<Object*>& Get() const
        {
            return objects;
        }

        void RequestUpdate(ANA_OBJECTS_UPDATE_FLAG_BIT flag = ANA_OBJECTS_UPDATE_ALL)
        {
            if (flag & ANA_OBJECTS_UPDATE_COMMAND_BUFFER)
                commandBufferNeedUpdate = true;
            if (flag & ANA_OBJECTS_UPDATE_UNIFORM_BUFFER)
                uniformBufferNeedUpdate = true;
        }

        const bool BeginCommandBufferUpdate()
        {
            return commandBufferNeedUpdate;
        }

        void EndCommandBufferUpdate()
        {
            commandBufferNeedUpdate = false;
        }

        const bool BeginUniformBufferUpdate()
        {
            return uniformBufferNeedUpdate;
        }

        void EndUniformBufferUpdate()
        {
            uniformBufferNeedUpdate = false;
        }

        void Append(Object* newObject)
        {
            objects.push_back(newObject);
            RequestUpdate();
        }

        void RemoveAt(int index)
        {
            int i = 0;
            for (auto obj = objects.begin(); obj < objects.end(); obj++)
            {
                if (i == index)
                {
                    objects.erase(obj);
                    RequestUpdate();
                    break;
                }

                i++;
            }
        }
    private:
        std::vector<Object*> objects;
        bool commandBufferNeedUpdate = false;
        bool uniformBufferNeedUpdate = false;
    };
}