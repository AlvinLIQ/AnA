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
            TextBlock(const char* newText, glm::vec4 color = {DefaultFontColor});
            ~TextBlock();

            void PrepareDraw(Shape* shapeBuffer, uint32_t& shapeCount) override;

            void Text(const char* newText);
            const char* Text();

            void Insert(size_t index, uint32_t ch);

            bool IsWrapping;
            Float FontSize = 14.0f;
            glm::vec4 FontColor{DefaultFontColor};
            void ApplyRenderInfo(Shape* shapeBuffer, uint32_t& shapeCount) override;
        private:
            uint32_t asciiLen = 0;
            uint32_t wideLen = 0;
        protected:
            uint32_t id = uint32_t(-1);
        };
    }
}
