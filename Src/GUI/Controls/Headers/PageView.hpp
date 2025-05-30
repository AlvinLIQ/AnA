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
            void PageIndex(uint32_t _pageIndex);
            uint32_t PageIndex();
        protected:
            std::vector<Control*> pages;
            uint32_t pageIndex = 0u;
        };
    }
}