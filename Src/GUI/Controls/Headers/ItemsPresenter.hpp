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

            void PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount);
            virtual void PointerEventTrigger(PointerEventArgs& args);
        protected:
            std::vector<Control*> items;
        };
    }
}