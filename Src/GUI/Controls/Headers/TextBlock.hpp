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

            VkDescriptorImageInfo GetDescriptorImageInfo() override;
            bool IsWrapping;
        protected:
            String text = "";
            Texture* texture{nullptr};
        };
    }
}