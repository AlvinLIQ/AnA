#pragma once

#include "Control.hpp"
#include <optional>

namespace AnA
{
    namespace Controls
    {
        class SceneView : public Control
        {
        public:
            SceneView();
            ~SceneView();
            void ImageInfo(VkDescriptorImageInfo _imageInfo)
            {
                imageInfo = _imageInfo;
            }

            VkDescriptorImageInfo GetDescriptorImageInfo();
        private:
            std::optional<VkDescriptorImageInfo> imageInfo;
        };
    }
}