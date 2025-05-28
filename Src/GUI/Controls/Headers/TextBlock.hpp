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

            virtual void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;

            void Text(const char* newText);
            const char* Text();

            bool IsWrapping;
            Float FontSize = 25.0f;
            glm::vec4 FontColor{0.0f, 0.0f, 0.0f, 1.0f};
            virtual void ApplyRenderInfo(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount) override;
        private:
            
        protected:
            uint32_t id = -1u;
        };
    }
}