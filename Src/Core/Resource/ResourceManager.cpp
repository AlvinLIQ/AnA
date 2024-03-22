#include "Headers/ResourceManager.hpp"

using namespace AnA;
using namespace Resource;

ResourceManager* _resourceManager = nullptr;

ResourceManager::ResourceManager()
{
    _resourceManager = this;
}

ResourceManager::~ResourceManager()
{

}

ResourceManager* ResourceManager::GetCurrent()
{
    return _resourceManager;
}

void ResourceManager::UpdateCamera(float aspect)
{
    MainCameraInfo.aspect = aspect;
    MainCameraInfo.UpdateCameraPerspective(MainCamera);
    LightCameraInfo.aspect = aspect;
    LightCameraInfo.UpdateCameraPerspective(LightCamera);
    //LightCamera.SetOrthographicProjection(-aspect, -1.0, aspect, 1.0, LightCameraInfo.near, LightCameraInfo.far);
}

void ResourceManager::initCamera()
{
    
}