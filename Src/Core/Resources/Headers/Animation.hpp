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

    enum AnimaionType{ANA_TRANSLATION = 1, ANA_ROATE = 2, ANA_SCALE = 4};
    struct Animation
    {
        std::vector<KeyFrame> keyFrames{};
        int type{};
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
