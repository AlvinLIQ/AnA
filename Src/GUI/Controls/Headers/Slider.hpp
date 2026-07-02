#pragma once

#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class Slider : public Control
        {
        public:
            Slider();

            float Value()
            {
                return value;
            }
            void Value(float newValue)
            {
                if (value != newValue)
                {
                    value = newValue;
                    if(OnValueChanged)
                        OnValueChanged(value);
                    RequestUpdate();
                }
            }
            Orientations Orientation{Horizontal};
            void ApplyRenderInfo(Shape* shapeBuffer, uint32_t& shapeCount) override;
            FloatValueCallback OnValueChanged = nullptr;
        private:
            Control button{};
            float value = 0.5f;
        };
    }
}
