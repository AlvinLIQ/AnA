#pragma once

#include "Camera.hpp"
#include "../../Input/Headers/InputManager.hpp"
#include <glm/detail/qualifier.hpp>
#include <vector>
namespace AnA
{
    namespace Cameras
    {
        class CameraController
        {
        public:
            struct CameraCallbackParam
            {
                Camera &aCamera;
                int id;
            };

            void GetCameraKeyMapConfigs(std::vector<Input::InputManager::KeyMapConfig> &configs);
            void GetCameraCursorConfigs(std::vector<Input::InputManager::CursorConfig> &configs);
            CameraController(Camera &mCamera);
            
            static void Move(CameraCallbackParam* param);
            static void Rotate(CameraCallbackParam* param);

            static void CursorMoved(Camera* camera, Input::InputManager::CursorPosition &duration);

        private:
            Camera &aCamera;
            std::vector<CameraCallbackParam> movements;
        };
    }
}