#pragma once
#include "../../../Core/Headers/AnA_Object.h"

namespace AnA
{
    namespace Controls
    {
        enum AlignType
        {
            Absolute, Relative
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
        //typedef void (*)(AnA_Control* control) PointerEventHandler;

        class AnA_Control : public AnA_Object
        {
        public:
            AnA_Control();
            ~AnA_Control();

            AlignType RenderMode {Relative};
            
        private:
            
        };
    }
}