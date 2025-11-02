#pragma once

#include "../../Headers/Types.hpp"
#include <vector>

namespace AnA
{
    struct KeyFrame
    {
        Transform transform;
    };

    struct Animation
    {
    public:
        uint32_t Position;
        std::vector<KeyFrame> KeyFrames{};
    };
}