#pragma once
#include "../../../Core/Headers/Types.hpp"

namespace AnA
{
    enum AlignType
    {
        Absolute, Relative, Auto
    };
    enum AlignmentType
    {
        Start, Center, End, Stretch
    };

    enum PointerEventType
    {
        Pressed, Entered, Released, Exited, Moved, Moving
    };
    enum PointerTriggerType
    {
        Touch, Mouse
    };
    struct PointerEventArgs
    {
        CursorPosition Position;
        PointerEventType EventType;
        PointerTriggerType TriggerType;
        bool Handled = false;
    };
    typedef void (*PointerEventHandler)(void* control, PointerEventArgs& args);
    struct SIZE_F
    {
        float Width;
        float Height;
    };
    struct POS_F
    {
        float x;
        float y;
    };
}