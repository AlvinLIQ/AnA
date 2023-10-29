#pragma once

#include "Control.hpp"

namespace AnA
{
    namespace Controls
    {
        class TextBlock : public Control
        {
        public:
            TextBlock();
            ~TextBlock();

            void PrepareDraw();

            void Text(const char* newText);
            const char* Text();

            bool IsWrapping;
        protected:
            std::string text = "";
        };
    }
}