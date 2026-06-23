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
        Pressed, Entered, Released, Exited, Moved, Moving, Scrolled, DragBegan, Dragging, DragEnded, PointerEventTypeCount
    };
    enum PointerTriggerType
    {
        Touch, Mouse
    };
    struct PointerEventArgs
    {
        CursorPosition Position;
        CursorPosition Duration;
        PointerEventType EventType;
        PointerTriggerType TriggerType;
        bool Handled = false;
    };
    typedef void (*PointerEventHandler)(void* control, PointerEventArgs& args);
}
