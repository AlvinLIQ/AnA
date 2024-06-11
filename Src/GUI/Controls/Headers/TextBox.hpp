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
            void Insert(size_t index, uint32_t ch);
        protected:
            std::string text = "";
            std::vector<AnA::String> stringBuffers;
            size_t cursor = 0;
            void CharacterRecevied(uint32_t ch);
        };
    }
}