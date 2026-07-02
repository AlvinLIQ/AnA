#pragma once

#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class ToggleSwitch : public Control
        {
        public:
            ToggleSwitch();

            void Toggle(bool _toggled);
            bool Toggle() const
            {
                return toggled;
            }
            void (*Toggled)(bool toggled) = nullptr;
            //void ApplyRenderInfo(Shape* shapeBuffer, uint32_t& shapeCount) override;
        private:
            bool toggled = false;
            static void ToggleSwitch_Toggled(ToggleSwitch* toggleSwitch, PointerEventArgs& );
        };
    }
}
