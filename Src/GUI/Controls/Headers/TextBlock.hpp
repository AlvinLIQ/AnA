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
            TextBlock(const char* newText, glm::vec4 color = {0.0f, 0.0f, 0.0f, 1.0f});
            ~TextBlock();

            void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;

            void Text(const char* newText);
            const char* Text();

            void Insert(size_t index, uint32_t ch);

            bool IsWrapping;
            Float FontSize = 16.0f;
            glm::vec4 FontColor{0.0f, 0.0f, 0.0f, 1.0f};
            void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:

        protected:
            uint32_t id = uint32_t(-1);
        };
    }
}
