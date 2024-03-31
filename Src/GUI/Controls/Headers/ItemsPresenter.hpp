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

            void PrepareDraw();

        protected:
            std::vector<Control*> items;
        };
    }
}