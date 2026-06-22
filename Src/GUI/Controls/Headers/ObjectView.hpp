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
        uint32_t subId;
        void *data;
    };

    namespace Controls
    {
        class ObjectViewItem : public StackPanel
        {
        public:
            ObjectViewItem(ObjectViewItemData data, uint32_t iconLayer = 0) : StackPanel(), Data{data}
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
                    image->TextureLayer = iconLayer;
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

            void AddItem(ObjectViewItemData itemData, uint32_t iconLayer = 0);
        };
    }
}
