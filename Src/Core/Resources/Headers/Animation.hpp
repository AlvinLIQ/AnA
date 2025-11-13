#pragma once

#include "../../Headers/Types.hpp"
#include <vector>
#include <string>

namespace AnA
{
    struct KeyFrame
    {
        Transform transform;
    };

    struct Animation
    {
        uint32_t position;
        std::vector<KeyFrame> keyFrames{};
        std::string name;
    };
}