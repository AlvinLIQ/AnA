#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


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

    private:
        glm::mat4 projectionMatrix{1.f};
        glm::mat4 viewMatrix{1.f};
    };
}