#pragma once

#include "TextBlock.hpp"

namespace AnA
{
    namespace Controls
    {
        class TextBox : public TextBlock
        {
        public:
            TextBox();
            void TextReceived(const char* text) override;
            void Focus() override;
            void Unfocus() override;
        protected:
            size_t cursor = 0;
        };
    }
}
