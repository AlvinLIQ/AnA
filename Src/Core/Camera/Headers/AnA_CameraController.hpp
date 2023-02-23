#pragma once

#include "AnA_Camera.hpp"
#include "../../Input/Headers/AnA_InputManager.hpp"
#include <vector>
namespace AnA
{
    namespace Camera
    {
        class AnA_CameraController
        {
        public:
            struct CameraCallbackParam
            {
                AnA_Camera &aCamera;
                glm::vec3 position;
            };

            void GetCameraKeyMapConfigs(std::vector<Input::AnA_InputManager::KeyMapConfig> &configs);
            AnA_CameraController(AnA_Camera &mCamera);
            
            static void Move(CameraCallbackParam *param);
            static void Rotate();
        private:
            AnA_Camera &aCamera;
            std::vector<CameraCallbackParam> movements;
        };
    }
}