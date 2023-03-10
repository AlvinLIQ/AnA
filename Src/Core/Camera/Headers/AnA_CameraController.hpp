#pragma once

#include "AnA_Camera.hpp"
#include "../../Input/Headers/AnA_InputManager.hpp"
#include <glm/detail/qualifier.hpp>
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
                int id;
            };

            void GetCameraKeyMapConfigs(std::vector<Input::AnA_InputManager::KeyMapConfig> &configs);
            void GetCameraCursorConfigs(std::vector<Input::AnA_InputManager::CursorConfig> &configs);
            AnA_CameraController(AnA_Camera &mCamera);
            
            static void Move(CameraCallbackParam *param);
            static void Rotate(CameraCallbackParam *param);

            static void CursorMoved(AnA_Camera *camera, Input::AnA_InputManager::CursorPosition &duration);

        private:
            AnA_Camera &aCamera;
            std::vector<CameraCallbackParam> movements;
        };
    }
}