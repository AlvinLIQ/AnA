#pragma once

#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class ItemPresenter : public Control
        {
        public:
            ItemPresenter();
            ~ItemPresenter();
            
            void Child(Control* newItem)
            {
                if (item != nullptr)
                    delete item;

                item = newItem;
            }

        protected:
            Control* item;
        };
    }
}