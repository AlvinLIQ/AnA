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
            
            void Child(Control* newItem);

            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfo, uint32_t& shapeCount);
        protected:
            Control* item{nullptr};
        };
    }
}