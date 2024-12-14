#pragma once
#include "../../Core/Headers/App.hpp"
#include <unordered_map>

namespace AnA
{
    namespace Editors
    {
        class Editor : public App
        {
        public:
            Editor();
            ~Editor();
            void Init();
            Controls::Control* InitControl();
        private:
            static void loadModelButton_Click(void* control, PointerEventArgs& args);
            static void saveSceneButton_Click(void* control, PointerEventArgs& args);
            std::unordered_map<std::string, Controls::Control*> controlMap;
        protected:
            void onLoop();
        };
    }
}