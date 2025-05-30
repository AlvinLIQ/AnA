#pragma once

#include "Control.hpp"
#include <vulkan/vulkan_core.h>

namespace AnA
{
    namespace Controls
    {
        class ItemPresenter : public Control
        {
        public:
            ItemPresenter();
            ~ItemPresenter();
            
            virtual void Child(Control* newItem);
            Control* Child();

            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
            void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
            void PointerEventTrigger(PointerEventArgs& args) override;

            AlignmentType HorizontalContentAlignment{Center};
            AlignmentType VerticalContentAlignment{Center};
        protected:
            Control* item{nullptr};
        };
    }
}