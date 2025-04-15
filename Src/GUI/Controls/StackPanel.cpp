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
    GetSizeForRender();
    GetActualControlOffset();
    StackPanel::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

void StackPanel::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    SIZE_F maxSize = RenderSize();
    auto renderOffset = RenderOffset() + Padding;
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
    if (items.size() && ((float*)&renderOffset)[invO] >= 0.0f)
    {
        ((float*)&offset)[invO] = -1.0f;
    }
    ((float*)&offset)[invO] += ((float*)&renderOffset)[invO];
    if (((float*)&maxSize)[invO] < 1.0f)
    {
        ((float*)&offset)[invO] -= ((float*)&maxSize)[invO];
    }
    for (size_t i = 0; i < items.size(); i++)
    {
        items[i]->Aspect = Aspect;
        items[i]->Extent = Extent;
        auto _size = items[i]->GetSizeForRender();
        ((float*)&offset)[invO] += ((float*)&size)[invO] + ((float*)&_size)[invO] + Spacing;
        size = _size;
        auto align = invO ? (AlignmentType*)&items[i]->HorizontalAlignment : &items[i]->VerticalAlignment;//(items[i] + offsets[o]);
        switch (*align)
        {
        case Start:
            ((float*)&offset)[o] = ((float*)&size)[o] - ((float*)&maxSize)[o] + ((float*)&renderOffset)[o];
            break;
        case End:
            ((float*)&offset)[o] = ((float*)&maxSize)[o] - ((float*)&size)[o];
            break;
        case Stretch:
            ((float*)&size)[o] = ((float*)&maxSize)[o];
            [[fallthrough]];
        default:
            ((float*)&offset)[o] = ((float*)&renderOffset)[o] * 0.5f;
            break;
        }
        items[i]->RenderOffset(offset);
        items[i]->RenderSize(size);
        items[i]->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
        ((float*)&maxSize)[invO] = std::max(((float*)&maxSize)[invO] ,(((float*)&offset)[invO] - ((float*)&renderOffset)[invO] + ((float*)&size)[invO]));
    }
    RenderSize(maxSize);
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}