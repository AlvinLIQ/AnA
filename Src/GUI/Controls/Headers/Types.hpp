#pragma once

#include <cstdint>

namespace AnA
{
    enum AlignType
    {
        Absolute, Relative, Auto
    };
    enum AlignmentType
    {
        Start, Center, End
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
        uint32_t Position[2];
        PointerEventType EventType;
        PointerTriggerType TriggerType;
    };
    typedef void (*PointerEventHandler)(void* control, PointerEventArgs args);
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