#pragma once
#include "../../Headers/Types.hpp"
#include <glm/glm.hpp>

namespace AnA
{
    /*Control*/
    const AlignmentType ControlHorizontalAlignment = Start;
    const AlignmentType ControlVerticalAlignment = Start;

    const SIZE_2F ControlSize = {1.0f, 1.0f};

    const SIZE_2F ControlMinSize = {};

    const AlignType ControlRenderMode = Auto;
    /*Button*/
    
    /*
    const SIZE_2F ButtonMinSize = {0.1f, 0.06f};
    const AlignType ButtonRenderMode = Auto;*/
    const SIZE_2F ButtonMinAbsoluteSize = {80.f, 36.0f};
    const glm::vec4 ButtonBackgroundColor = {0.85f, 0.85f, 0.85f, 1.0f};
    const glm::vec4 ButtonPointerMovedBackgroundColor = {0.78f, 0.78, 0.78f, 1.0f};
    const glm::vec4 ButtonPointerPressedBackgroundColor = {0.68f, 0.68f, 0.68f, 1.0f};
}