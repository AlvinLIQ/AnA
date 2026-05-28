#include "Headers/Animation.hpp"
#include "Headers/Scene.hpp"
#include "glm/ext/scalar_constants.hpp"

using namespace AnA;

Animations::Animations()
{
    Animation animation;
    animation.name = "testScaleAnimation";
    animation.type = ANA_ROTATION | ANA_TRANSLATION | ANA_SCALE;

    KeyFrame keyframe, keyframe1{};
    keyframe.time = 0.0f;
    keyframe.transform.rotation = {};
    keyframe.transform.scale = {1.0f, 1.0f, 1.0f};
    keyframe.transform.translation = {0.0f ,0.0f ,0.0f};

    keyframe1.time = 2.0f;
    keyframe1.transform.scale = {2.0f, 2.0f, 2.0f};
    keyframe1.transform.translation = {1.0f, 1.0f, 1.0f};
    keyframe1.transform.rotation = glm::vec3(glm::pi<float>(), 0.0f, 0.0f);

    animation.keyFrames.push_back(keyframe);
    animation.keyFrames.push_back(keyframe1);

    keyframe.time = 4.0f;
    keyframe.transform.rotation = glm::vec3(glm::two_pi<float>(), 0.0f, 0.0f);
    animation.keyFrames.push_back(keyframe);

    animations.push_back(animation);
}

uint32_t Animations::Append(Animation& animation)
{
    animations.push_back(animation);
    return uint32_t(animations.size() - 1);
}

void Animations::Update(Scene& scene, float dt)
{
    auto& meshes = scene.meshes;

    for (size_t i = 0; i < meshes.size(); i++)
    {
        if (ProcessAnimationInfo(meshes[i].animationInfo, meshes[i].transform, dt))
            scene.UpdateMeshTransform(uint32_t(i));
    }
}

bool Animations::ProcessAnimationInfo(AnimationInfo& info, Transform& transform, float dt)
{
    if (!info.playing || info.id >= animations.size())
        return false;

    auto& animation = animations[info.id];
    info.time += dt;
    for (uint32_t j = info.pos + 1; j < animation.keyFrames.size(); j++)
    {
        auto& keyFrame0 = animation.keyFrames[j - 1];
        auto& keyFrame1 = animation.keyFrames[j];
        float factor = (info.time - keyFrame0.time)
            / (keyFrame1.time - keyFrame0.time);

        if (info.time >= keyFrame1.time)
        {
            info.pos++;
            continue;
        }

        if (animation.type & ANA_SCALE)
            transform.scale = keyFrame0.transform.scale +
                factor * (keyFrame1.transform.scale - keyFrame0.transform.scale);
        if (animation.type & ANA_ROTATION)
            transform.rotation = keyFrame0.transform.rotation +
                factor * (keyFrame1.transform.rotation - keyFrame0.transform.rotation);
        if (animation.type & ANA_TRANSLATION)
            transform.translation = keyFrame0.transform.translation +
                factor * (keyFrame1.transform.translation - keyFrame0.transform.translation);
        //scene.UpdateMeshTransform(uint32_t(i));
        break;
    }

    if (info.pos + 1 >= animation.keyFrames.size())
    {
        info.pos = 0;
        info.time = 0;
        if (!info.loop)
            info.playing = false;
    }

    return true;
}
