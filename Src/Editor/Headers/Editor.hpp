#pragma once
#include "../../Core/Headers/App.hpp"

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
        private:
            static void loadModelButton_Click(void* control, PointerEventArgs& args);
        };
    }
}