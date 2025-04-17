#pragma once
#include "CommandBuffer.hpp"

namespace AnA
{
    class Renderable
    {
        virtual void Draw(CommandBuffer& commandBuffer) = 0;
        virtual void DrawIndirect(CommandBuffer& commandBuffer) = 0;
        virtual void Update() = 0;
        virtual bool NeedUpdate() = 0;
    };
}