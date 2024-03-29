#pragma once
#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class ItemsPresenter : public Control
        {
        public:
            ItemsPresenter();
            ~ItemsPresenter();
            
            void AppendItem(Control* newItem)
            {
                items.push_back(newItem);
            }
            void RemoveItem(Control* targetItem)
            {
                if (targetItem == NULL)
                    return;

                for (auto item = items.begin(); item < items.end(); item++)
                {
                    if (*item == targetItem)
                    {
                        items.erase(item);
                        delete targetItem;

                        break;
                    }
                }
            }
            void RemoveItemAt(int index)
            {
                if (index >= items.size())
                    return;

                Control* targetItem = items[index];
                items.erase(items.begin() + index);
                delete targetItem;
            }

            void Draw(VkCommandBuffer commandBuffer);

        protected:
            std::vector<Control*> items;
        };
    }
}