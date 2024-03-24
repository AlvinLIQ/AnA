#pragma once
#include "../../Headers/Types.hpp"
#include <glm/glm.hpp>

namespace AnA
{
    /*Control*/
    const AlignmentType ControlHorizontalAlignment = Start;
    const AlignmentType ControlVerticalAlignment = Start;

    const SIZE_F ControlSize = {1.0f, 1.0f};

    const SIZE_F ControlMinSize = {};

    const AlignType ControlRenderMode = Absolute;
    /*Button*/
    const SIZE_F ButtonMinSize = {10.f, 10.0f};
    const AlignType ButtonRenderMode = Relative;
    const glm::vec3 ButtonBackgroundColor = {0.85f, 0.85f, 0.85f};
    const glm::vec3 ButtonPointerMovedBackgroundColor = {0.78f, 0.78, 0.78f};
    const glm::vec3 ButtonPointerPressedBackgroundColor = {0.68f, 0.68f, 0.68f};
}