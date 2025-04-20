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
        
            bool Toggled = false;
            //void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:
        };
    }
}
