#pragma once
#include "../../Headers/Types.hpp"
#include "../../Headers/Device.hpp"
#include "Descriptor.hpp"

namespace AnA
{
    struct Shape
    {
        glm::mat4 transform{1.0f};
        alignas(8) glm::vec3 color{1.0f};
    };
    struct ShapeInfo
    {
        Transform Transform{};
        glm::vec3 Color{1.0f};
    };
    class Shapes
    {
    public:
        Shapes(Device* mDeivce);
        ~Shapes();
        void PrepareDraw(Controls::Control* control);
        void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        void DrawIndirect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        VkOffset2D Offset;
        VkExtent2D Extent;
    private:
        Device* aDevice;
        Buffer* shapeBuffer{nullptr};
        Buffer* indirectBuffer{nullptr};
        Descriptor* ssboDescriptor{nullptr};
        uint32_t shapeCount{};
    };
}