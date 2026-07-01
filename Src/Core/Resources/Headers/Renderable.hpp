#pragma once
#include "CommandBuffer.hpp"

namespace AnA
{
    class Renderable
    {
    public:
        virtual void Bind(CommandBuffer& commandBuffer, Shader& shader) = 0;
        virtual void Draw(CommandBuffer& commandBuffer) = 0;
        virtual void DrawIndirect(CommandBuffer& commandBuffer) = 0;
        virtual void Update() = 0;
        virtual bool NeedUpdate() = 0;
        VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    };
}
