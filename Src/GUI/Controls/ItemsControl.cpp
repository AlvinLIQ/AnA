#include "Headers/ItemsControl.hpp"

using namespace AnA;
using namespace Controls;

ItemsControl::ItemsControl() : Control()
{

}

ItemsControl::~ItemsControl()
{
    for (auto item = items.begin(); item < items.end(); item++)
    {
        Control* pItem = *item;
        items.erase(item);
        delete pItem;
    }
}