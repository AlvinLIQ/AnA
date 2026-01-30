#pragma once
#include "../../Core/Headers/App.hpp"
#include <unordered_map>

#define EDITOR_LEFT_PANEL_WIDTH 350

namespace AnA
{
    struct ObjectViewItemData;
    namespace Editor
    {
        class EditorApp : public App
        {
        public:
            EditorApp();
            virtual ~EditorApp();
            void Init();
            Controls::Control* InitControl();
            // 0 normal 1 grab 2 rotate 3 scale
            char ActionMode = 0;
            ObjectViewItemData* SelectedObjectData = nullptr;
        private:
            static void loadModelButton_Click(void* control, PointerEventArgs& args);
            static void saveSceneButton_Click(void* control, PointerEventArgs& args);
            static void exitButton_Click(void* control, PointerEventArgs& args);
            static void pageButton_Click(void* , PointerEventArgs& );
            static void mainScene_MeshAppend(std::string name, uint32_t id);
            template<char actionMode>
            Input::RegularCallBack createActionModeCallback();
            std::unordered_map<std::string, Controls::Control*> controlMap;
        protected:
            static void onLoop();
        };
    }
}
