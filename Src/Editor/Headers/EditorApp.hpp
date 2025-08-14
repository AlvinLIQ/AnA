#pragma once
#include "../../Core/Headers/App.hpp"
#include <unordered_map>

#define EDITOR_LEFT_PANEL_WIDTH 350

namespace AnA
{
    namespace Editor
    {
        class EditorApp : public App
        {
        public:
            EditorApp();
            virtual ~EditorApp();
            void Init();
            Controls::Control* InitControl();
        private:
            static void loadModelButton_Click(void* control, PointerEventArgs& args);
            static void saveSceneButton_Click(void* control, PointerEventArgs& args);
            static void exitButton_Click(void* control, PointerEventArgs& args);
            static void pageButton_Click(void* , PointerEventArgs& );
            std::unordered_map<std::string, Controls::Control*> controlMap;
        protected:
            static void onLoop();
        };
    }
}