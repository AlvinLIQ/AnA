#pragma once

#include "StackPanel.hpp"

namespace AnA
{
    namespace Controls
    {
        class ListView : public StackPanel
        {
        public:
            ListView();
            int SelectionIndex() const
            {
                return selectionIndex;
            }
            void Select(int index);
            void Select(Control* item);

            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;

        private:
            int selectionIndex = -1;
            Control* selectedItem = nullptr;
            Control* hoverItem = nullptr;
            static void ListView_PointerMoving(ListView* control, PointerEventArgs& args);
            static void ListView_PointerExited(ListView* control, PointerEventArgs& args);
        };
    }
}
