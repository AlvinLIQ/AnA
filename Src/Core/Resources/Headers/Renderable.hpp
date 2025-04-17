#pragma once
#include "CommandBuffer.hpp"

namespace AnA
{
    class Renderable
    {
    public:
        virtual void Bind(CommandBuffer& commandBuffer, uint32_t bufferIndex) = 0;
        virtual void Draw(CommandBuffer& commandBuffer) = 0;
        virtual void DrawIndirect(CommandBuffer& commandBuffer) = 0;
        virtual void Update() = 0;
        virtual bool NeedUpdate() = 0;
    };
}