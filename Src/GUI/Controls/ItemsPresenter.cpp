#include "Headers/ItemsPresenter.hpp"

using namespace AnA;
using namespace Controls;

ItemsPresenter::ItemsPresenter() : Control()
{

}

ItemsPresenter::~ItemsPresenter()
{
    for (auto item = items.begin(); item < items.end(); item++)
    {
        Control* pItem = *item;
        items.erase(item);
        delete pItem;
    }
}