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
        alignas(4) uint32_t textureId{0};
    };
    struct ShapeInfo
    {
        AnA::Transform Transform{};
        glm::vec3 Color{1.0f};
    };
    class Shapes
    {
    public:
        Shapes()
        {

        }
        Shapes(Device* mDeivce);
        Shapes(const Shapes&) = delete;
        Shapes& operator=(const Shapes&) = delete;
        Shapes(Shapes&& shapes) noexcept : aDevice{shapes.aDevice}, shapeBuffer{shapes.shapeBuffer}, shapeCount{shapes.shapeCount}, indirectBuffer{shapes.indirectBuffer}, ssboDescriptor{shapes.ssboDescriptor}
        {
            shapes.ssboDescriptor = nullptr;
            shapes.indirectBuffer = nullptr;
            shapes.shapeBuffer = nullptr;
            shapes.shapeCount = 0;
        }
        Shapes& operator=(Shapes&& shapes) noexcept
        {
            if (&shapes != this)
            {
                Shapes::~Shapes();
                aDevice = shapes.aDevice;
                shapeBuffer = shapes.shapeBuffer;
                shapeCount = shapes.shapeCount;
                indirectBuffer = shapes.indirectBuffer;
                ssboDescriptor = shapes.ssboDescriptor;

                shapes.ssboDescriptor = nullptr;
                shapes.indirectBuffer = nullptr;
                shapes.shapeBuffer = nullptr;
                shapes.shapeCount = 0;
                
            }
            return *this;
        }
        ~Shapes();
        void PrepareDraw(Controls::Control* control);
        void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        void DrawIndirect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        VkOffset2D Offset;
        VkExtent2D Extent;
    private:
        Device* aDevice{nullptr};
        Buffer* shapeBuffer{nullptr};
        Buffer* indirectBuffer{nullptr};
        Descriptor* ssboDescriptor{nullptr};
        uint32_t shapeCount{};
    };
}