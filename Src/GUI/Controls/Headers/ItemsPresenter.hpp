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

            virtual void Child(Control* newItem);
            virtual void Child(Control* newItem, uint32_t x, uint32_t y) = 0;
            
            void RemoveChild(Control* targetItem);
            void RemoveChildAt(size_t index);

            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
            void PointerEventTrigger(PointerEventArgs& args) override;
            float Spacing = 0.0f;
            Vec2 Padding = {};
        protected:
            std::vector<Control*> items;
        };
    }
}