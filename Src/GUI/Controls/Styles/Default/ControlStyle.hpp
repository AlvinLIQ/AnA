#pragma once
#include "../../Headers/Types.hpp"
#include <glm/glm.hpp>

namespace AnA
{
    /*Control*/
    const AlignmentType ControlHorizontalAlignment = Start;
    const AlignmentType ControlVerticalAlignment = Start;

    const Vec2 ControlSize = {1.0f, 1.0f};

    const Vec2 ControlMinSize = {};

    const AlignType ControlRenderMode = Auto;
    /*Button*/
    
    /*
    const Vec2 ButtonMinSize = {0.1f, 0.06f};
    const AlignType ButtonRenderMode = Auto;*/
    const Vec2 ButtonMinAbsoluteSize = {80.f, 36.0f};
    const glm::vec4 ButtonBackgroundColor = {0.85f, 0.85f, 0.85f, 1.0f};
    const glm::vec4 ButtonPointerMovedBackgroundColor = {0.78f, 0.78, 0.78f, 1.0f};
    const glm::vec4 ButtonPointerPressedBackgroundColor = {0.68f, 0.68f, 0.68f, 1.0f};
}