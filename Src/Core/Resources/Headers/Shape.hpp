#pragma once
#include "../../Headers/Types.hpp"
#include "../../Headers/Device.hpp"
#include "../../Headers/Buffer.hpp"
#include "Renderable.hpp"
#include "Descriptor.hpp"

namespace AnA
{
    struct Shape
    {
        glm::mat4 transform{1.0f};
        glm::vec4 color{1.0f};
        uint32_t texLayer{0};
    };
    struct ShapeInfo
    {
        AnA::Transform Transform{};
        glm::vec4 Color{1.0f};
        uint32_t TextureId{0};
        uint32_t TextureLayer{0};
        uint32_t shapeId;
    };
    class Shapes : public Renderable
    {
    public:
        Shapes()
        {

        }
        Shapes(Device* mDeivce);
        Shapes(const Shapes&) = delete;
        Shapes& operator=(const Shapes&) = delete;
        Shapes(Shapes&& shapes) noexcept : aDevice{shapes.aDevice}, shapeBuffer{std::move(shapes.shapeBuffer)}, indirectBuffer{std::move(shapes.indirectBuffer)}, ssboDescriptor{shapes.ssboDescriptor}
        {
            shapeCount = shapes.shapeCount;
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
                shapeBuffer = std::move(shapes.shapeBuffer);
                shapeCount = shapes.shapeCount;
                indirectBuffer = std::move(shapes.indirectBuffer);
                countBuffer = std::move(shapes.countBuffer);
                ssboDescriptor = shapes.ssboDescriptor;
                samplersDescriptor = shapes.samplersDescriptor;

                shapes.samplersDescriptor = nullptr;
                shapes.ssboDescriptor = nullptr;
                shapes.shapeCount = 0;
                
            }
            return *this;
        }
        virtual ~Shapes();
        void PrepareDraw(Controls::Control* control);
        void Bind(CommandBuffer& commandBuffer, Shader& shader, uint32_t bufferIndex) override;
        void Draw(CommandBuffer& commandBuffer) override;
        void DrawIndirect(CommandBuffer& commandBuffer) override;
        void Update() override
        {
            
        }
        bool NeedUpdate() override;
        VkOffset2D Offset;
        VkExtent2D Extent;
    private:
        Device* aDevice{nullptr};
        Buffer shapeBuffer{};
        Buffer indirectBuffer{};
        Buffer countBuffer{};
        Descriptor* ssboDescriptor{nullptr};
        Descriptor* samplersDescriptor{nullptr};
        uint32_t shapeCount{};
        std::vector<VkDescriptorImageInfo> imageInfos{};
        VkDescriptorSet sets[2];
        bool updated = false;
    };
}