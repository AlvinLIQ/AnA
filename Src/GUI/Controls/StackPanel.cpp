#include "Headers/StackPanel.hpp"

using namespace AnA;
using namespace Controls;

StackPanel::StackPanel()
{
    RenderMode(Absolute);
}

StackPanel::~StackPanel()
{
}

void StackPanel::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    auto RenderOffset = ControlOffset;
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
        items[i]->Extent = Extent;
        auto _size = items[i]->GetSizeForRender();
        ((float*)&offset)[invO] += ((float*)&size)[invO] + ((float*)&_size)[invO] + Spacing + ((float*)&RenderOffset)[invO];
        size = _size;
        auto align = invO ? (AlignmentType*)&items[i]->HorizontalAlignment : &items[i]->VerticalAlignment;//(items[i] + offsets[o]);
        switch (*align)
        {
        case Start:
            ((float*)&offset)[o] = ((float*)&size)[o] - maxSize + ((float*)&RenderOffset)[o];
            break;
        case End:
            ((float*)&offset)[o] = maxSize - ((float*)&size)[o];
            break;
        case Stretch:
            ((float*)&size)[o] = maxSize;
        default:
            ((float*)&offset)[o] = ((float*)&RenderOffset)[o] * 0.5f;
            break;
        }
        items[i]->RenderOffset(offset);
        items[i]->RenderSize(size);
        items[i]->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    }

    Control::PrepareDraw(shapeBuffer, imageInfos, shapeCount);
}
