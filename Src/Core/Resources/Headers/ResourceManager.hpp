#pragma once
#include "../../Camera/Headers/Camera.hpp"

namespace AnA
{
    namespace Resource
    {
        class ResourceManager
        {
        public:
            ResourceManager();
            ~ResourceManager();

            static ResourceManager* GetCurrent();

            //Built-in resources
            Cameras::Camera MainCamera;
            Cameras::CameraInfo MainCameraInfo{glm::radians(60.0f), 1.0, 0.01f, 100.0f};
            Cameras::Camera LightCamera;
            Cameras::CameraInfo LightCameraInfo{glm::radians(45.0f), 1.0, 0.5f, 96.0f};

            void UpdateCamera(float aspect);
        private:
            void initCamera();
        };
    }
}