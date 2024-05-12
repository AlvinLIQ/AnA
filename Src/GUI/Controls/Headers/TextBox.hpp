#pragma once

#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class TextBox : Control
        {
        public:
            TextBox();
            std::string Text()
            {
                return text;
            }
            void Text(const std::string& newText)
            {
                text = newText;
            }
        protected:
            std::string text = "";
            std::vector<AnA::String> stringBuffers;
        };
    }
}