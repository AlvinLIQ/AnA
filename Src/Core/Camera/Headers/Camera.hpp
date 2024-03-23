#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../../Headers/Types.hpp"

#define moveStep 1.0f
#define rotateStep 0.8f
namespace AnA
{
    namespace Cameras
    {
        struct CameraBufferObject
        {
            glm::mat4 proj{1.f};
            glm::mat4 view{1.f};
            glm::mat4 invView{1.f};
            glm::vec2 resolution{};
        };
        class Camera
        {
        public:
            void SetOrthographicProjection(float left, float top, float right, float bottom, float near, float far);
            void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

            void AddViewOffset(glm::vec3 position);

            const glm::mat4 &GetProjectionMatrix() const
            {
                return projectionMatrix;
            }
            const glm::mat4 &GetView() const
            {
                return viewMatrix;
            }
            const glm::mat4 &GetInverseView() const
            {
                return inverseViewMatrix;
            }

            void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.0f, -1.0f , 0.0f });
            void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f, -1.0f , 0.0f });
            void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

            void SetSpeedRatio(float newRatio)
            {
                speedRatio = newRatio;
            }
            const float GetSpeedRatio() const
            {
                return speedRatio;
            }

            glm::vec3 offset{};
            Transform CameraTransform;
            void UpdateViewMatrix();
        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
            glm::mat4 inverseViewMatrix;

            float speedRatio = 1.0f;
        };

        struct CameraInfo
        {
            float fov;
            float aspect;
            float near;
            float far;
            void UpdateCameraPerspective(Camera& camera)
            {
                camera.SetPerspectiveProjection(fov, aspect, near, far);
            }
        };
    }
}