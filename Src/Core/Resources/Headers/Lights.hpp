#pragma once
#include <glm/glm.hpp>
#include "../../Headers/Device.hpp"

namespace AnA
{
    namespace Lights
    {
        struct LightBufferObject
        {
            glm::mat4 proj{1.f};
            glm::mat4 view{1.f};
            alignas(16) glm::vec3 direction{0.0f};
            alignas(16) glm::vec3 color{};
            float ambient{0.0f};
        };

        struct PointLight
        {
            glm::vec3 position;
            glm::vec3 color;
        };

        struct Ray
        {
            glm::vec3 origin;
            glm::vec3 direction;
        };
        
        class Light
        {
        public:
            Light(Device& mDevice);
            ~Light();

            glm::vec3 Direction{1.0f, -3.0f, 1.0f};
            glm::vec3 Color{0.2};
            float Ambient{0.37f};
            Buffer** GetBuffers();
            void UpdateBuffers(Cameras::Camera& lightCamera);
        private:
            Device& aDevice;
            std::vector<Buffer*> buffers;
            void createBuffers();
        };
    }
}