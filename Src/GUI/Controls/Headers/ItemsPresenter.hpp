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
            virtual void Child(Control* , uint32_t , uint32_t )
            {

            }

            void RemoveChild(Control* targetItem);
            virtual void RemoveChildAt(size_t index);

            std::vector<Control*> Children()
            {
                return items;
            }

            void PointerEventTrigger(PointerEventArgs& args) override;
            float Spacing = 0.0f;
            Vec4 Padding = {};
        protected:
            std::vector<Control*> items;
        };
    }
}
