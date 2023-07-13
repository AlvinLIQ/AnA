#pragma once
#include "../../Headers/Types.hpp"

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
}