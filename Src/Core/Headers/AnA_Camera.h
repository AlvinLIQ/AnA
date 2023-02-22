#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define MOVESTEP 0.05
namespace AnA
{
    class AnA_Camera
    {
    public:
        void SetOrthographicProjection(float left, float top, float right, float bottom, float near, float far);
        void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

        void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
        void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
        void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4 &GetProjectionMatrix() const
        {
            return projectionMatrix;
        }
        const glm::mat4 &GetView() const
        {
            return viewMatrix;
        }

        void MoveForward()
        {
            viewMatrix[3].z -= MOVESTEP;
        }
        void MoveBack()
        {
            viewMatrix[3].z += MOVESTEP;
        }
        void MoveLeft()
        {
            viewMatrix[3].x += MOVESTEP;
        }
        void MoveRight()
        {
            viewMatrix[3].x -= MOVESTEP;
        }
        void MoveUp()
        {
            viewMatrix[3].y += MOVESTEP;
        }
        void MoveDown()
        {
            viewMatrix[3].y -= MOVESTEP;
        }
    private:
        glm::mat4 projectionMatrix{1.f};
        glm::mat4 viewMatrix{1.f};
    };
}