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
    const glm::vec4 MaxColor = {1.0f, 1.0f, 1.0f, 2.0f};

    /*
    const Vec2 ButtonMinSize = {0.1f, 0.06f};
    const AlignType ButtonRenderMode = Auto;*/
    const Vec2 ButtonMinAbsoluteSize = {80.f, 36.0f};
#if ANA_THEME==0
    const glm::vec4 DefaultPrimaryColor = {0.1f, 0.1f, 0.1f, 1.0f};
    const glm::vec4 DefaultSecondaryColor = {0.156f, 0.156f, 0.156f, 1.0f};
    const glm::vec4 DefaultControlBackgroundColor = {0.2f, 0.2f, 0.2f, 1.0f};
    const glm::vec4 DefaultFontColor = {0.8f, 0.8f, 0.8f, 1.0f};
    const glm::vec4 DefaultListItemHoverColor = {0.25f, 0.25f, 0.25f, 1.0f};
    const glm::vec4 DefaultListItemSelectedColor = {0.3f, 0.3f, 0.3f, 1.0f};
    const glm::vec4 ButtonBackgroundColor = {0.2f, 0.2f, 0.2f, 1.0f};
    const glm::vec4 ButtonPointerMovedBackgroundColor = {0.22f, 0.22, 0.22f, 1.0f};
    const glm::vec4 ButtonPointerPressedBackgroundColor = {0.18f, 0.18f, 0.18f, 1.0f};

#elif ANA_THEME==1
    const glm::vec4 DefaultPrimaryColor = {0.88f, 0.88f, 0.88f, 1.0f};
    const glm::vec4 DefaultSecondaryColor = {0.956f, 0.956f, 0.956f, 1.0f};
    const glm::vec4 DefaultControlBackgroundColor = {0.84f, 0.84f, 0.84f, 1.0f};
    const glm::vec4 DefaultFontColor = {0.2f, 0.2f, 0.2f, 1.0f};
    const glm::vec4 DefaultListItemHoverColor = {0.9f, 0.9f, 0.9f, 1.0f};
    const glm::vec4 DefaultListItemSelectedColor = {0.83f, 0.83f, 0.83f, 1.0f};
    const glm::vec4 ButtonBackgroundColor = {0.8f, 0.8f, 0.8f, 1.0f};
    const glm::vec4 ButtonPointerMovedBackgroundColor = {0.7f, 0.7f, 0.7f, 1.0f};
    const glm::vec4 ButtonPointerPressedBackgroundColor = {0.6f, 0.6f, 0.6f, 1.0f};
#endif
}
