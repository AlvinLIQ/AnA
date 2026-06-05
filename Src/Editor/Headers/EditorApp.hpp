#pragma once
#include "../../Core/Headers/App.hpp"
#include <unordered_map>

#define EDITOR_LEFT_PANEL_WIDTH 350

namespace AnA
{
    struct ObjectViewItemData;
    namespace Editor
    {
        enum class ActionModes {Normal, Grab, Rotate, Scale};
        enum class AxisType {All = 3, X = 0, Y = 1, Z = 2};
        class EditorApp : public App
        {
        public:
            EditorApp();
            virtual ~EditorApp();
            void Init();
            Controls::Control* InitControl();
            // 0 normal 1 grab 2 rotate 3 scale
            ActionModes ActionMode = ActionModes::Normal;
            // 0 all 1 x 2 y 3 z
            AxisType FocusedAxis = AxisType::All;
            ObjectViewItemData* SelectedObjectData = nullptr;
        private:
            static void loadModelButton_Click(void* control, PointerEventArgs& args);
            static void saveSceneButton_Click(void* control, PointerEventArgs& args);
            static void exitButton_Click(void* control, PointerEventArgs& args);
            static void pageTabs_SelectionChanged(void*);
            static void mainScene_MeshAppend(std::string name, uint32_t id);
            template<ActionModes actionMode>
            Input::RegularCallBack createActionModeCallback();
            template<AxisType axisType>
            Input::RegularCallBack createAxisTypeCallback();
            std::unordered_map<std::string, Controls::Control*> controlMap;
        protected:
            static void onLoop();
        };
    }
}
