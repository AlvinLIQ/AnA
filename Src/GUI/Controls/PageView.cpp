#include "Headers/PageView.hpp"

using namespace AnA;
using namespace Controls;

PageView::PageView()
{

}

PageView::~PageView()
{
    for (auto& page : pages)
        delete page;
    item = nullptr;
}

void PageView::Child(Control* newItem)
{
    pages.push_back(newItem);
}