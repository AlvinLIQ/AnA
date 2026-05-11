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
            //void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:
            bool toggled = false;
            static void ToggleSwitch_Toggled(ToggleSwitch* toggleSwitch, PointerEventArgs& );
        };
    }
}
