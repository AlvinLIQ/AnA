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
            TextBlock(const char* newText, glm::vec3 color = {});
            ~TextBlock();

            virtual void PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount);

            void Text(const char* newText);
            const char* Text();

            VkDescriptorImageInfo GetDescriptorImageInfo();
            bool IsWrapping;
        protected:
            String text = "";
            Texture* texture{nullptr};
        };
    }
}