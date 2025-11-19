#include "../Core/Headers/App.hpp"

class benchmark : public AnA::App
{
public:
    benchmark() : AnA::App()
    {
        for (uint32_t i = 4; i < 10; i++)
        {
            aResourceManager.TextureMap.try_emplace(i, 
                uint32_t((random() & 0x00FFFFFFu) ^ 0xFF000000u), &aDevice);
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
                    {}}, uint32_t(random() % 10)});
            }
        }
        scene.Append(meshInfos);
    }
protected:
    virtual void onCommandBufferRecording(AnA::CommandBuffer& commandBuffer) override
    {
        auto& swapChain = aRenderer.GetSwapChain();
        aRenderer.BeginRendering(commandBuffer);
        swapChain.SetViewport(commandBuffer);
        aRenderSystem.RenderIndirect(commandBuffer, aResourceManager.MainScene,
            aDevice.MeshShaderSupport() ? aResourceManager.Shaders[5] : aResourceManager.Shaders[0],
            swapChain.CurrentFrame);
        aRenderer.EndRendering(commandBuffer);
    }
};