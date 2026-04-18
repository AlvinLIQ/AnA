#pragma once

#include "../../Headers/Types.hpp"
#include <vector>
#include <string>

namespace AnA
{
    struct KeyFrame
    {
        float time;
        Transform transform;
    };

    struct Animation
    {
        std::vector<KeyFrame> keyFrames{};
        std::string name;
    };

    struct AnimationInfo
    {
        uint32_t id;
        float time = 0.0f;
        bool loop = false;
    };

    class Scene;

    class Animations
    {
    public:
        Animations();

        void Update(Scene& scene, float dt);

    private:
        std::vector<Animation> animations;
    };
}
