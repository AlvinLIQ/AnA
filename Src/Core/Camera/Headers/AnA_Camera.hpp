#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../../Headers/AnA_Types.hpp"

#define moveStep 1.0f
#define rotateStep 0.8f
namespace AnA
{
    namespace Camera
    {
        class AnA_Camera
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

            float speedRatio = 1.0f;
        };
    }
}