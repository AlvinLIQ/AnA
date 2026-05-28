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

    enum AnimationType{ANA_TRANSLATION = 1, ANA_ROTATION = 2, ANA_SCALE = 4};
    enum Interpolation{ANA_LINEAR, ANA_CUBIC_SPLINE};
    struct Animation
    {
        std::vector<KeyFrame> keyFrames{};
        int type{};
        Interpolation interpolation{ANA_LINEAR};
        std::string name;
    };

    struct AnimationInfo
    {
        uint32_t id = 0xFFFFFFFF;
        uint32_t pos;
        float time = 0.0f;
        bool loop = false;
        bool playing = false;
    };

    class Scene;

    class Animations
    {
    public:
        Animations();

        void Update(Scene& scene, float dt);
        bool ProcessAnimationInfo(AnimationInfo& info, Transform& transform, float dt);
    private:
        std::vector<Animation> animations;
    };
}
