#pragma once
#include "../../GUI/Controls/Headers/Control.hpp"
#include <unordered_map>
#include <string>
#include <vector>

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

    class ObjectView
    {
    public:
        ObjectView();
        ~ObjectView();

        Controls::Control* InitControl();
    private:
        std::vector<ObjectViewItemData> items;
        std::unordered_map<std::string, Controls::Control*> controlMap;
    };
}
