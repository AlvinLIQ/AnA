#include "Headers/StackPanel.hpp"

using namespace AnA;
using namespace Controls;

StackPanel::StackPanel()
{
    SetRenderMode(Absolute);
}

StackPanel::~StackPanel()
{
    
}

void StackPanel::PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount)
{
    float maxSize = 1.0f;
    int o = Orientation, invO = 1 - Orientation;
    /*
    for (int i = 0; i < items.size(); i++)
    {
        auto size = items[i]->GetSizeForRender();
        if (((float*)&size)[o] > maxSize)
            maxSize = ((float*)&size)[o];
    }*/
    SIZE_F size{};
    POS_F offset{};
    if (items.size())
    {
        ((float*)&offset)[invO] = -1.0;
    }
    for (int i = 0; i < items.size(); i++)
    {
        items[i]->Aspect = Aspect;
        auto _size = items[i]->GetSizeForRender();
        ((float*)&offset)[invO] += ((float*)&size)[invO] + ((float*)&_size)[invO];
        size = _size;
        auto align = invO ? (AlignmentType*)&items[i]->HorizontalAlignment : &items[i]->VerticalAlignment;//(items[i] + offsets[o]);
        switch (*align)
        {
        case Start:
            ((float*)&offset)[o] = ((float*)&size)[o] - maxSize;
            break;
        case End:
            ((float*)&offset)[o] = maxSize - ((float*)&size)[o];
            break;
        case Stretch:
            ((float*)&size)[o] = maxSize;
        default:
            ((float*)&offset)[o] = 0.0f;
            break;
        }
        items[i]->SetRenderOffset(offset);
        items[i]->SetRenderSize(size);
        items[i]->Transform.translation = {offset.x, offset.y, 0.0f};
        items[i]->Transform.scale = {size.Width, size.Height, 1.0f};
        shapeBuffer[shapeCount].transform = items[i]->Transform.mat4();
        shapeBuffer[shapeCount].color = items[i]->Color;
        shapeCount++;
    }

    Control::PrepareDraw(shapeBuffer, shapeCount);
}
