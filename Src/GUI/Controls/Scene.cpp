#include "Headers/Scene.hpp"

using namespace AnA;
using namespace Controls;

Scene::Scene()
{
    RenderMode(Relative);
}

Scene::~Scene()
{

}

VkDescriptorImageInfo Scene::GetDescriptorImageInfo()
{
    if (imageInfo.has_value())
        return imageInfo.value();
    else
        return Control::GetDescriptorImageInfo();
}