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

void Animations::Update(Scene& scene, float dt)
{
    auto& meshes = scene.meshes;

    for (size_t i = 0, j; i < meshes.size(); i++)
    {
        if (meshes[i].animationID >= animations.size())
            continue;

        auto& animation = animations[meshes[i].animationID];
        meshes[i].animationTime += dt;
        for (j = meshes[i].animationPos + 1; j < animation.keyFrames.size(); j++)
        {
            auto& keyFrame0 = animation.keyFrames[j - 1];
            auto& keyFrame1 = animation.keyFrames[j];
            float factor = (meshes[i].animationTime - keyFrame0.time)
                        / (keyFrame1.time - keyFrame0.time);

            if (meshes[i].animationTime >= keyFrame1.time)
                meshes[i].animationPos++;

            if (animation.type & ANA_SCALE)
                meshes[i].transform.scale = keyFrame0.transform.scale +
                    factor * (keyFrame1.transform.scale - keyFrame0.transform.scale);
            if (animation.type & ANA_ROATE)
            {
                meshes[i].transform.rotation = keyFrame0.transform.rotation +
                    factor * (keyFrame1.transform.rotation - keyFrame0.transform.rotation);
                printf("%f\r", meshes[i].transform.rotation.x);
            }
            if (animation.type & ANA_TRANSLATION)
                meshes[i].transform.translation = keyFrame0.transform.translation +
                    factor * (keyFrame1.transform.translation - keyFrame0.transform.translation);
            scene.UpdateMeshTransform(i);
            break;
        }

        if (meshes[i].animationPos + 1 >= animation.keyFrames.size())
        {
            meshes[i].animationPos = 0;
            meshes[i].animationTime = 0;
            if (!meshes[i].loop)
                meshes[i].animationID = 0xFFFFFFFF;
        }
    }
}
