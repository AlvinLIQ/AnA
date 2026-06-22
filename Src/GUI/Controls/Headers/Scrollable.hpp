#pragma once
#include "ItemPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        class Scrollable : public ItemPresenter
        {
        public:
            Scrollable();
            void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:
            static void scrolled(void*, AnA::PointerEventArgs& args);
        };
    }
}
