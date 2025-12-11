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
            void CharacterRecevied(uint32_t ch) override;
        protected:
            size_t cursor = 0;
        };
    }
}