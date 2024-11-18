#pragma once
#include "../../Headers/Types.hpp"
#include "../../Headers/Device.hpp"
#include "../../Headers/Buffer.hpp"
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
        AnA::Transform Transform{};
        glm::vec3 Color{1.0f};
        uint32_t TextureId{0};
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
            shapes.samplersDescriptor = nullptr;
            shapes.ssboDescriptor = nullptr;
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
                countBuffer = shapes.countBuffer;
                ssboDescriptor = shapes.ssboDescriptor;
                samplersDescriptor = shapes.samplersDescriptor;

                shapes.samplersDescriptor = nullptr;
                shapes.ssboDescriptor = nullptr;
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
        Buffer shapeBuffer;
        Buffer indirectBuffer;
        Buffer countBuffer;
        Descriptor* ssboDescriptor{nullptr};
        Descriptor* samplersDescriptor{nullptr};
        uint32_t shapeCount{};
        std::vector<VkDescriptorImageInfo> imageInfos{};
    };
}