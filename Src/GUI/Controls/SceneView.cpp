#include "Headers/SceneView.hpp"

using namespace AnA;
using namespace Controls;

SceneView::SceneView()
{
    RenderMode(Relative);
}

SceneView::~SceneView()
{

}

VkDescriptorImageInfo SceneView::GetDescriptorImageInfo()
{
    if (imageInfo.has_value())
        return imageInfo.value();
    else
        return Control::GetDescriptorImageInfo();
}