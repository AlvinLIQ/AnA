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

        const glm::mat4 &GetProjectionMatrix() const
        {
            return projectionMatrix;
        }
    private:
        glm::mat4 projectionMatrix{1.f};
    };
}