#pragma once

#include "ItemPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        class PageView : public ItemPresenter
        {
        public:
            PageView();
            ~PageView();
            virtual void Child(Control* newItem) override;
        protected:
            std::vector<Control*> pages;
            uint32_t pageIndex = 0u;
        };
    }
}