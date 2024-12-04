#include "Headers/Scene.hpp"

using namespace AnA;
using namespace Controls;

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::PrepareDraw()
{
    
}

VkDescriptorImageInfo Scene::GetDescriptorImageInfo()
{
    if (imageInfo.has_value())
        return imageInfo.value();
    else
        return Control::GetDescriptorImageInfo();
}