#pragma once
#include "ItemsPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        class StackView : public ItemsPresenter
        {
        public:
            StackView();
            ~StackView();
            void PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount) override;
            void ApplyRenderInfo(Shape* shapeBuffer, uint32_t& shapeCount) override;
        };
    }
}
