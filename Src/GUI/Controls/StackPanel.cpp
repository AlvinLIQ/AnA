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
    StackPanel::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}

Vec2 StackPanel::GetSizeForRender()
{
    Control::GetSizeForRender();
    GetActualControlOffset();
    Vec2 maxSize = RenderSize();
    int o = Orientation, invO = 1 - Orientation;
    Vec2 size{};
    Vec2 offset{};
    float* size2F = reinterpret_cast<float*>(&size);
    float* offset2F = reinterpret_cast<float*>(&offset);
    float* maxSize2F = reinterpret_cast<float*>(&maxSize);
    float* renderOffset2F = reinterpret_cast<float*>(&renderOffset);
    if (items.size() && renderOffset2F[invO] >= 0.0f)
    {
        offset2F[invO] = -1.0f;
    }
    offset2F[invO] += renderOffset2F[invO];
    if (maxSize2F[invO] < 1.0f)
    {
        offset2F[invO] -= maxSize2F[invO];
    }
    for (size_t i = 0; i < items.size(); i++)
    {
        items[i]->Aspect = Aspect;
        items[i]->Extent = Extent;
        auto _size = items[i]->GetSizeForRender();
        offset2F[invO] += size2F[invO] + reinterpret_cast<float*>(&_size)[invO] + Spacing;
        size = _size;
        auto align = invO ? items[i]->HorizontalAlignment : items[i]->VerticalAlignment;//(items[i] + offsets[o]);
        switch (align)
        {
        case Start:
            offset2F[o] = size2F[o] - maxSize2F[o] + renderOffset2F[o];
            break;
        case End:
            offset2F[o] = maxSize2F[o] - size2F[o];
            break;
        case Stretch:
            size2F[o] = maxSize2F[o];
            [[fallthrough]];
        case Center:
            [[fallthrough]];
        default:
            offset2F[o] = renderOffset2F[o] * 0.5f;
            break;
        }
        items[i]->RenderOffset(offset);
        items[i]->RenderSize(size);
        maxSize2F[invO] = std::max(maxSize2F[invO] ,(offset2F[invO] - renderOffset2F[invO] + size2F[invO]));
    }
    RenderSize(maxSize);
    return maxSize;
}

void StackPanel::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    for (auto& item : items)
    {
        item->RenderOffset() += renderOffset + Padding;
        item->ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
    }
    Control::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}
