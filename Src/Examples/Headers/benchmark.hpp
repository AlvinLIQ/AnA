#include "../Core/Headers/App.hpp"
#include <stdlib.h>

class benchmark : public AnA::App
{
public:
    benchmark() : AnA::App("Benchmarker")
    {
        aResourceManager.TextureMap.try_emplace(4, 
                "Textures/building1.jpg", &aDevice);
        aResourceManager.TextureMap.try_emplace(5, 
                "Textures/building2.jpg", &aDevice);
        for (uint32_t i = 6; i < 10; i++)
        {
            aResourceManager.TextureMap.try_emplace(i, 
                uint32_t((rand() & 0x00FFFFFFu) ^ 0xFF000000u), &aDevice);
        }
        auto& scene = aResourceManager.MainScene;
        std::vector<AnA::MeshInfo> meshInfos{
            {"Models/cube.obj", 
                {{0.0f, -0.3f, 0.0f}, 
                {103.0f, 0.15f, 103.0f},
                {}}, 0}
        };
        for (float i = 0.0f, j; i <= 1.0f; i += 0.033f)
        {
            for (j = 0.0f; j <= 1.0f; j += 0.033f)
            {
                float h = (float(random_double()) * 7.0f) + 2.0f;
                meshInfos.push_back({"Models/cube.obj", 
                    {{i * 200.0f - 100.0f, h, j * 200.0f - 100.0f},
                    {1.0f + random_double(), h + 0.3f, 1.0f + random_double()},
                    {}}, uint32_t(rand() % 10)});
            }
        }
        aResourceManager.MainCamera.CameraTransform.translation.x = 2.5f;
        scene.Append(meshInfos);
    }

    void Init()
    {
        aInputManager.GlobalProfile.flag = AnA::Input::InputProfileFlags::None;
        AnA::Input::InputProfile profile{};
        profile.flag = AnA::Input::InputProfileFlags::None;
        aInputManager.GetProfiles().push_back(profile);
        aInputManager.SetActiveProfile(1);        
    }
private:
    float totalTime = 0.0f;
    glm::vec3 direction = glm::vec3{1.0f};
    float rotateYAnimationResult = 0.0f;
    bool rotationStarted = false;
protected:
    virtual void onCommandBufferRecording(AnA::CommandBuffer& commandBuffer) override
    {
        aResourceManager.MainCamera.CameraTransform.translation.y = sinf(totalTime) + 1.5f;
        aResourceManager.MainCamera.offset.z += 5.5f * direction.z;
        if (rotationStarted)
        {
            aResourceManager.MainCamera.CameraTransform.rotation.y += frameTime * glm::pi<float>();
            if (aResourceManager.MainCamera.CameraTransform.rotation.y >= rotateYAnimationResult)
            {
                aResourceManager.MainCamera.CameraTransform.rotation.y = rotateYAnimationResult;
                while (aResourceManager.MainCamera.CameraTransform.rotation.y >= glm::two_pi<float>())
                    aResourceManager.MainCamera.CameraTransform.rotation.y -= glm::two_pi<float>();
                rotationStarted = false;
            }
        }
        if (aResourceManager.MainCamera.CameraTransform.translation.z >= 95.0f)
        {
            direction.z = -1.0f;
            rotationStarted = true;
            rotateYAnimationResult = glm::pi<float>();
        }
        else if (aResourceManager.MainCamera.CameraTransform.translation.z <= -95.0f)
        {
            direction.z = 1.0f;
            rotationStarted = true;
            rotateYAnimationResult = glm::two_pi<float>();
        }
        aResourceManager.MainCamera.UpdateViewMatrix();
        totalTime += frameTime;
        auto& swapChain = aRenderer.GetSwapChain();
        aRenderer.BeginRendering(commandBuffer);
        swapChain.SetViewport(commandBuffer);
        aRenderSystem.RenderIndirect(commandBuffer, aResourceManager.MainScene,
            aDevice.MeshShaderSupport() ? aResourceManager.Shaders[5] : aResourceManager.Shaders[0],
            swapChain.CurrentFrame);
        aRenderer.EndRendering(commandBuffer);
    }
};