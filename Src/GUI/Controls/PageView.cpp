#include "Headers/PageView.hpp"

using namespace AnA;
using namespace Controls;

PageView::PageView()
{
}

PageView::~PageView()
{
    item = nullptr;
    for (auto& page : pages)
        delete page;
}

void PageView::Child(Control* newItem)
{
    pages.push_back(newItem);
    if (!item)
        PageIndex(pageIndex);
}

void PageView::PageIndex(uint32_t _pageIndex)
{
    if (_pageIndex < uint32_t(pages.size()))
    {
        pageIndex = _pageIndex;
        item = pages[pageIndex];
        RequestTextLayoutReset();
    }
}

uint32_t PageView::PageIndex()
{
    return pageIndex;
}
