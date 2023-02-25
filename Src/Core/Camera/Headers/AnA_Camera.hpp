#pragma once

#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../../Headers/AnA_Types.hpp"

#define MOVESTEP 0.05
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

            void ApplyRotation(glm::vec3 &position, glm::vec3 &rotation);

            glm::vec3 offset{};
            Transform CameraTransform;
            void UpdateViewMatrix()
            {
                //ApplyRotation(offset, CameraTransform.rotation);
                CameraTransform.translation -= offset;
                offset = CameraTransform.mat4()[3];
                SetViewYXZ({}, CameraTransform.rotation);
                viewMatrix[3] += glm::vec4(offset, 0.f);
                offset = {};
            }
        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
        };
    }
}