#pragma once
#include "ItemPresenter.hpp"
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
        ObjectViewItemType type;
        uint32_t id;
        void *data;
    };

    namespace Controls
    {
        class ObjectViewItem : public ItemPresenter
        {
        public:
            ObjectViewItem(ObjectViewItemData data) : ItemPresenter(), Data{data}
            {
                RenderMode(Absolute);
                Color = {};
                ControlSize = {10.0f, 25.0f};
                item = new TextBlock((std::to_string(Data.id) + " " + Data.name).c_str());
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
