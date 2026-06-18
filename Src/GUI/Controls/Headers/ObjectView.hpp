#pragma once
#include "TextBlock.hpp"
#include "ListView.hpp"
#include "Types.hpp"
#include <string>

namespace AnA
{
    enum ObjectViewItemType { ANA_OBJECT_TYPE_MODEL = 0, ANA_OBJECT_TYPE_TEXT = 1, ANA_OBJECT_TYPE_IMAGE = 2};
    struct ObjectViewItemData
    {
        std::string name;
        std::string icon;
        ObjectViewItemType type;
        uint32_t id;
        void *data;
    };

    namespace Controls
    {
        class ObjectViewItem : public StackPanel
        {
        public:
            ObjectViewItem(ObjectViewItemData data) : StackPanel(), Data{data}
            {
                RenderMode(Absolute);
                Color = {};
                ControlSize = {10.0f, 25.0f};
                if (!data.icon.empty())
                {
                    auto image = new Control;
                    image->RenderMode(Absolute);
                    image->ControlSize = {64.0f, 64.0f};
                    image->Texture(data.icon);
                    Child(image);
                }
                auto itemTextBlock = new TextBlock((std::to_string(Data.id) + " " + Data.name).c_str());
                itemTextBlock->VerticalAlignment = AlignmentType::Center;
                Child(itemTextBlock);
            }
            ObjectViewItemData Data;
        };
        class ObjectView : public Controls::ListView
        {
        public:
            ObjectView();
            ~ObjectView();

            void AddItem(ObjectViewItemData itemData);
        };
    }
}
