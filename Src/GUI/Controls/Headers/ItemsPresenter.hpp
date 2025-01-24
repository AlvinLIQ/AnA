#pragma once
#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class ItemsPresenter : public Control
        {
        public:
            ItemsPresenter();
            ~ItemsPresenter();

            void Child(Control* newItem);
            
            void RemoveChild(Control* targetItem);
            void RemoveChildAt(int index);

            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount);
            virtual void PointerEventTrigger(PointerEventArgs& args);
            float Spacing = 0.0f;
            POS_F Padding = {};
        protected:
            std::vector<Control*> items;
        };
    }
}