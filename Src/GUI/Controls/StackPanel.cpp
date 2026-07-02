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

void StackPanel::PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount)
{
    GetSizeForRender();
    StackPanel::ApplyRenderInfo(shapeBuffer, shapeCount);
}

Vec2 StackPanel::GetSizeForRender()
{
    RenderSize({});
    Control::GetSizeForRender();
    GetActualControlOffset();
    Vec2 maxSize = RenderSize();
    int o = Orientation, invO = 1 - Orientation;
    Vec2 size{};
    Vec2 offset{};
    float* size2F = reinterpret_cast<float*>(&size);
    float* offset2F = reinterpret_cast<float*>(&offset);
    float* maxSize2F = reinterpret_cast<float*>(&maxSize);

    offset = {Padding.x() * 0.5f, Padding.y() * 0.5f};

    for (size_t i = 0; i < items.size(); i++)
    {
        items[i]->Aspect = Aspect;
        items[i]->Extent = Extent;
        auto _size = items[i]->GetSizeForRender();
        size = _size;

        items[i]->RenderOffset(offset);
        items[i]->RenderSize(size);

        maxSize2F[invO] = std::max(maxSize2F[invO], (offset2F[invO] + size2F[invO]) + (Padding[invO] + Padding[2 + invO]) * 0.5f);
        maxSize2F[o] = std::max(maxSize2F[o], size2F[o]);
        offset2F[invO] += size[invO];
        offset2F[invO] += Spacing * 0.5f;
    }
    RenderSize(maxSize);
    return maxSize;
}

Vec2 StackPanel::ContentRenderSize()
{
    return RenderSize() - Vec2(Padding.x(), Padding.y()) * 0.5f - Vec2(Padding.z(), Padding.w()) * 0.5f;
}

void StackPanel::ApplyRenderInfo(Shape* shapeBuffer, uint32_t& shapeCount)
{
    int o = Orientation, invO = 1 - Orientation;
    float offset = renderOffset[invO];
    for (auto& item : items)
    {
        AlignmentType aligns[2] = {item->HorizontalAlignment, item->VerticalAlignment};//(items[i] + offsets[o]);
        if (aligns[invO] == Stretch && item == items.back())
            items.back()->RenderSize()[invO] = renderSize[invO] - items.back()->RenderOffset()[invO];

        item->RenderOffset()[invO] += offset;
        switch (aligns[o])
        {
        case Center:
            item->RenderOffset()[o] = renderOffset[o] + (renderSize[o] - item->RenderSize()[o]) * 0.5f - Padding[o] * 0.5f;
            break;
        case End:
            item->RenderOffset()[o] = renderSize[o] - item->RenderSize()[o] + renderOffset[o] - Padding[2 + 0] * 0.5f;
            break;
        case Stretch:
            item->RenderSize()[o] = renderSize[o] - Padding[o] * 0.5f - Padding[2 + o] * 0.5f;
            [[fallthrough]];
        case Start:
            [[fallthrough]];
        default:
            item->RenderOffset()[o] = renderOffset[o] + Padding[o] * 0.5f;
            break;
        }
        item->ApplyRenderInfo(shapeBuffer, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, shapeCount);
}
