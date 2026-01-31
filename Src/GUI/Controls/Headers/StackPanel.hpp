#pragma once
#include "ItemsPresenter.hpp"

namespace AnA
{
    namespace Controls
    {
        class StackPanel : public ItemsPresenter
        {
        public:
            StackPanel();
            ~StackPanel();

            Orientations Orientation {Horizontal};
            Vec2 GetSizeForRender() override;
            Vec2 ContentRenderSize() override;
            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
            void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:
        };
    }
}
