#pragma once
#include "ItemsPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        enum Orientations {Horizontal = 1, Vertical = 0};
        class StackPanel : public ItemsPresenter
        {
        public:
            StackPanel();
            ~StackPanel();

            Orientations Orientation {Horizontal};
            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfo, uint32_t& shapeCount);
        private:
        };
    }
}