#include "Headers/Animation.hpp"
#include "Headers/Scene.hpp"

using namespace AnA;

Animations::Animations()
{

}

void Animations::Update(Scene& scene, float dt)
{
    auto& meshes = scene.meshes;

    for (size_t i = 0, j; i < meshes.size(); i++)
    {
        if (meshes[i].animationID >= animations.size())
            continue;

        auto& animation = animations[i];
        meshes[i].animationTime += dt;
        for (j = meshes[i].animationPos + 1; j < animation.keyFrames.size(); j++)
        {
            auto& keyFrame0 = animation.keyFrames[j - 1];
            auto& keyFrame1 = animation.keyFrames[j];
            float factor = (meshes[i].animationTime - keyFrame0.time)
                        / (keyFrame1.time - keyFrame0.time);
            if (factor > 1.0f)
            {
                continue;
            }
            meshes[i].animationPos = uint32_t(j);

            meshes[i].transform.scale = keyFrame0.transform.scale +
                factor * (keyFrame1.transform.scale - keyFrame0.transform.scale);
            meshes[i].transform.rotation = keyFrame0.transform.rotation +
                factor * (keyFrame1.transform.rotation - keyFrame0.transform.rotation);
            meshes[i].transform.translation = keyFrame0.transform.translation +
                factor * (keyFrame1.transform.translation - keyFrame0.transform.translation);
            scene.UpdateMeshTransform(i);
            break;
        }

        if (meshes[i].animationPos >= animation.keyFrames.size())
        {
            meshes[i].animationPos = 0;
            meshes[i].animationTime = 0;
            if (!meshes[i].loop)
                meshes[i].animationID = 0xFFFFFFFF;
        }
    }
}
