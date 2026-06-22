#include "Headers/Scrollable.hpp"

using namespace AnA;
using namespace Controls;

void Scrollable::scrolled(void* param, AnA::PointerEventArgs& args)
{
    auto scrollable = static_cast<Scrollable*>(param);
    scrollable->itemOffset += {args.Duration.x.As<float>(), args.Duration.y.As<float>()};
    glm::vec2 maxScroll = {1.0f - scrollable->renderSize.x() - scrollable->renderOffset.x(),
        1.0f - scrollable->renderSize.y() - scrollable->renderOffset.y()};
    scrollable->itemOffset.x() = std::min(std::max(scrollable->itemOffset.x().value, maxScroll.x), 0.0f);
    scrollable->itemOffset.y() = std::min(std::max(scrollable->itemOffset.y().value, maxScroll.y), 0.0f);
    RequestUpdate();
}

Scrollable::Scrollable() : ItemPresenter()
{
    PointerEvents[PointerEventType::Scrolled].push_back(scrolled);
}

void Scrollable::ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    ItemPresenter::ApplyRenderInfo(shapeBuffer, imageInfos, shapeCount);
}
