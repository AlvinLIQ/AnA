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
            Control* SelectedItem() const
            {
                return selectedItem;
            }
            void Select(int index);
            void Select(Control* item);

            void RemoveChildAt(size_t index) override;

            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
            Input::RegularCallBack SelectionChanged{nullptr};
        private:
            int selectionIndex = -1;
            Control* selectedItem = nullptr;
            Control* hoverItem = nullptr;
            static void ListView_PointerMoving(ListView* control, PointerEventArgs& args);
            static void ListView_PointerPressed(ListView* control, PointerEventArgs& args);
            static void ListView_PointerExited(ListView* control, PointerEventArgs& args);
        };
    }
}
