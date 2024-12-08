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
        
            float Value = 0.5f;
            Orientations Orientation{Horizontal};
            void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount);
        private:
            Control button{};
        };
    }
}
