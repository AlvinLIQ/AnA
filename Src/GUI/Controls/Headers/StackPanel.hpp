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
            virtual void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
            virtual void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:
        };
    }
}