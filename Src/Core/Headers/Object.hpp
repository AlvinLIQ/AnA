#pragma once

#include "Model.hpp"
#include "Texture.hpp"
#include "Types.hpp"
#include "glm/fwd.hpp"
#include "vulkan/vulkan_core.h"
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#define SHAPE_TYPE uint32_t
#define ANA_TRIANGLE 0
#define ANA_RECTANGLE 1
#define ANA_ELLIPSE 2
#define ANA_CURVED_RECTANGLE 3
#define ANA_MODEL 4
#define ANA_TEXT 5

#define MAX_OBJECTS_COUNT 100

namespace AnA
{
    struct ObjectPushConstantData
    {
        glm::uint32_t sType;
        alignas(4) glm::uint32_t index;
        glm::vec3 color;
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
    };

    typedef char ANA_OBJECTS_UPDATE_FLAG_BIT;
    static const ANA_OBJECTS_UPDATE_FLAG_BIT ANA_OBJECTS_UPDATE_COMMAND_BUFFER = 1;
    static const ANA_OBJECTS_UPDATE_FLAG_BIT ANA_OBJECTS_UPDATE_STORAGE_BUFFER = 2;
    static const ANA_OBJECTS_UPDATE_FLAG_BIT ANA_OBJECTS_UPDATE_ALL = 3;

    typedef glm::vec<2, uint32_t> Range;

    class Objects
    {
    public:
        void Init(Device* mDevice, VkDescriptorSetLayout& descriptorSetLayout)
        {
            aDevice = mDevice;
            createObjectsBuffers();
            aDevice->CreateDescriptorPool(MAX_FRAMES_IN_FLIGHT, descriptorPool);
            aDevice->CreateDescriptorSets((std::vector<void*>&)objectsBuffers, MAX_OBJECTS_COUNT * sizeof(Model::ModelStorageBufferObject), 1, MAX_FRAMES_IN_FLIGHT, descriptorPool, descriptorSetLayout, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorSets);
        }

        void Destroy()
        {
            for (auto& object : objects)
                delete object;
            objects.clear();
            for (auto& objectsBuffer : objectsBuffers)
                delete objectsBuffer;
            delete staggingBuffer;

            vkDestroyDescriptorPool(aDevice->GetLogicalDevice(), descriptorPool, nullptr);
        }

        const std::vector<Object*>& Get() const
        {
            return objects;
        }
        
        std::vector<VkDescriptorSet>& GetDescriptorSets()
        {
            return descriptorSets;
        }

        void RequestUpdate(ANA_OBJECTS_UPDATE_FLAG_BIT flag = ANA_OBJECTS_UPDATE_ALL)
        {
            if (flag & ANA_OBJECTS_UPDATE_COMMAND_BUFFER)
                commandBufferNeedUpdate = true;
            if (flag & ANA_OBJECTS_UPDATE_STORAGE_BUFFER && objects.size())
                UpdateStorageBuffer({0, static_cast<uint32_t>(objects.size())});
        }

        void UpdateStorageBuffer(Range updateRange)
        {
            int y1 = updateRange.x + updateRange.y, y2;
            for (auto& updateQueueItem : updateQueue)
            {
                y2 = updateQueueItem.x + updateQueueItem.y;
                if (updateRange.x >= updateQueueItem.x)
                {
                    if (y1 <= y2)
                    {
                        return;
                    }
                    else if (updateRange.x <= y2)
                    {
                        updateQueueItem.y += y1 - y2;
                        updateRange.y = updateQueueItem.y;
                        return;
                    }
                }
                else if (y1 <= y2)
                {
                    if (y1 >= updateQueueItem.x)
                    {
                        updateQueueItem.x = updateRange.x;
                        updateQueueItem.y = y2 - updateRange.x;
                        updateRange.y = updateQueueItem.y;
                        return;
                    }
                }
                else
                {
                    updateQueueItem = updateRange;
                    if (updateRange.y > maxUpdateRange)
                        maxUpdateRange = updateRange.y;
                    return;
                }
            }

            updateQueue.push_back(updateRange);
        }

        const bool BeginCommandBufferUpdate()
        {
            return commandBufferNeedUpdate;
        }

        void EndCommandBufferUpdate()
        {
            commandBufferNeedUpdate = false;
        }

        const bool BeginStorageBufferUpdate()
        {
            return updateQueue.size();
        }

        void CommitStorageBufferUpdate(VkCommandBuffer& commandBuffer)
        {
            staggingBuffer->Map(0, sizeof(glm::mat4) * MAX_OBJECTS_COUNT);
            Model::ModelStorageBufferObject* staggingBufferData = (Model::ModelStorageBufferObject*)staggingBuffer->GetMappedData();

            std::vector<VkBufferCopy> regions;

            for (auto& updateRange : updateQueue)
            {
                VkBufferCopy region{};
                region.dstOffset = updateRange.x * sizeof(Model::ModelStorageBufferObject);
                region.srcOffset = region.dstOffset;
                region.size = updateRange.y * sizeof(Model::ModelStorageBufferObject);
                for (uint32_t i = 0; i < updateRange.y; i++)
                {
                    staggingBufferData[updateRange.x + i] = {objects[updateRange.x + i]->Properties.transform.mat4()};
                }
                regions.push_back(region);
            }
            for (auto& objectsBuffer : objectsBuffers)
                objectsBuffer->CopyToBuffer(*staggingBuffer, static_cast<uint32_t>(regions.size()), regions.data(), commandBuffer);
            staggingBuffer->Unmap();
        }

        void EndStorageBufferUpdate()
        {
            maxUpdateRange = 0;
            updateQueue.clear();
        }

        void Append(Object* newObject)
        {
            objects.push_back(newObject);
            UpdateStorageBuffer({objects.size() - 1, 1});
            RequestUpdate(ANA_OBJECTS_UPDATE_COMMAND_BUFFER);
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
        bool commandBufferNeedUpdate = false;

        Device* aDevice;

        std::vector<Object*> objects;
        std::vector<Range> updateQueue;
        uint32_t maxUpdateRange = 0;
        std::vector<Buffer*> objectsBuffers;
        Buffer* staggingBuffer;
        void createObjectsBuffers()
        {
            objectsBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            for (auto& objectsBuffer : objectsBuffers)
            {
                objectsBuffer = new Buffer(*aDevice, sizeof(glm::mat4) * MAX_OBJECTS_COUNT,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            }
            staggingBuffer = new Buffer(*aDevice, sizeof(glm::mat4) * MAX_OBJECTS_COUNT,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
        
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
        
    };
}