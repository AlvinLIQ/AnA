#include "Headers/StackPanel.hpp"

using namespace AnA;
using namespace Controls;

StackPanel::StackPanel()
{

}

StackPanel::~StackPanel()
{
    
}

void StackPanel::PrepareDraw()
{
    float maxSize = 0;
    int o = Orientation;
    Control* t = nullptr;
    size_t offsets[] = {(size_t)&t->HorizontalAlignment, (size_t)&t->VerticalAlignment};
    for (int i = 0; i < items.size(); i++)
    {
        auto size = items[i]->GetSizeForRender();
        if (size.Height > maxSize)
            maxSize = ((float*)&size)[o];
    }
    for (int i = 0; i < items.size(); i++)
    {
        auto size = items[i]->GetSizeForRender();
        auto align = (AlignmentType*)(items[i] + offsets[o]);
        if (*align == Start)
        {
            ((float*)&items[i]->ControlOffset)[o] = 0.0f;
        }
        else if (*align == Center)
        {
            ((float*)&items[i]->ControlOffset)[o] = (maxSize - ((float*)&items[i]->ControlSize)[o]) / 2.0f;
        }
        else
        {
            ((float*)&items[i]->ControlOffset)[o] = maxSize - ((float*)&items[i]->ControlSize)[o];
        }
    }
}
