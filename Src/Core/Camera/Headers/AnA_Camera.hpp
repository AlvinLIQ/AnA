#pragma once

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

            Transform CameraTransform;
            void UpdateViewMatrix()
            {
                viewMatrix = CameraTransform.mat4();
            }
        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
        };
    }
}