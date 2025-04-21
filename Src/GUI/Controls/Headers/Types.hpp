#pragma once
#include "../../../Core/Headers/Types.hpp"

namespace AnA
{
    enum AlignType
    {
        Absolute, Relative, Auto, Passive
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
    struct SIZE_2F
    {
        Float Width;
        Float Height;
    };
    struct POS_2F
    {
        Float x;
        Float y;
        POS_2F operator+(POS_2F& B)
        {
            return {x + B.x, y + B.y};
        }
    };
}