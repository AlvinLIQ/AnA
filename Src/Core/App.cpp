#include "Headers/App.hpp"
#include "Camera/Headers/CameraController.hpp"
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <chrono>

using namespace AnA;

std::shared_ptr<Model> _2DModel;
Device* _aDevice;
App* _aApp;

#define UI_SIGNAL_EXIT 0
#define UI_SIGNAL_KEY 2
#define UI_SIGNAL_WAIT 1

bool _uiLoopShouldEnd = false;
short _uiSignal;
int _uiParam[2];

App::App(const char* name) : aWindow(name),
    aInstance(aWindow),
    aInputManager(aWindow),
    aDevice(aInstance.GetInstance(), aWindow.GetSurface()),
    aRenderer(aWindow, &aDevice),
    aResourceManager(&aDevice)

{
    _aApp = this;
    _aDevice = &aDevice;
}

App::~App()
{
    Cleanup();
}

App* App::GetCurrent()
{
    return _aApp;
}

void App::CreateCubeModel(std::shared_ptr<Model>& model)
{
    std::vector<Model::Vertex> vertices =
    {
        // left (gray)
        {{-.5f, -.5f, -.5f},{-1.0f, 0.0f, 0.0f}},
        {{-.5f, -.5f, .5f}, {-1.0f, 0.0f, 0.0f}},
        {{-.5f, .5f, -.5f}, {-1.0f, 0.0f, 0.0f}},
        {{-.5f, .5f, .5f},  {-1.0f, 0.0f, 0.0f}},
        // right (red)
        {{.5f, -.5f, -.5f}, {1.0f, 0.0f, 0.0f}},
        {{.5f, -.5f, .5f},  {1.0f, 0.0f, 0.0f}},
        {{.5f, .5f, -.5f},  {1.0f, 0.0f, 0.0f}},
        {{.5f, .5f, .5f},   {1.0f, 0.0f, 0.0f}},
        // top (yellow)
        {{-.5f, -.5f, -.5f},{0.0f, -1.0f, 0.0f}},
        {{-.5f, -.5f, .5f}, {0.0f, -1.0f, 0.0f}},
        {{.5f, -.5f, -.5f}, {0.0f, -1.0f, 0.0f}},
        {{.5f, -.5f, .5f},  {0.0f, -1.0f, 0.0f}},
        // bottom (white)
        {{-.5f, .5f, -.5f}, {0.0f, 1.0f, 0.0f}},
        {{-.5f, .5f, .5f},  {0.0f, 1.0f, 0.0f}},
        {{.5f,  .5f, -.5f}, {0.0f, 1.0f, 0.0f}},
        {{.5f,  .5f, .5f},  {0.0f, 1.0f, 0.0f}},
        // back (green)
        {{-.5f, -.5f, .5f}, {0.0f, 0.0f, -1.0f}},
        {{-.5f, .5f, .5f},  {0.0f, 0.0f, -1.0f}},
        {{.5f,  -.5f, .5f}, {0.0f, 0.0f, -1.0f}},
        {{.5f,  .5f, .5f},  {0.0f, 0.0f, -1.0f}},
        // front (blue)
        {{-.5f, -.5f, -.5f}, {0.0f, 0.0f, 1.0f}},
        {{-.5f, .5f, -.5f},  {0.0f, 0.0f, 1.0f}},
        {{.5f,  -.5f, -.5f}, {0.0f, 0.0f, 1.0f}},
        {{.5f,  .5f, -.5f},  {0.0f, 0.0f, 1.0f}},
    };

    Model::ModelInfo modelInfo = {{}, vertices, {}, {}, {}, {0, 1, 2, 1, 2, 3}};
    model = std::make_shared<Model>(modelInfo);
}

void App::Init()
{
    //glfwSetKeyCallback(aWindow->GetGLFWwindow(), App::keyCallback);
    updateSceneOffset();
}

void App::Run()
{
    Cameras::Camera& camera = aResourceManager.MainCamera;
    Cameras::CameraController cameraController{camera};
    auto& activeProfile = aInputManager.GetActiveProfile();
    cameraController.GetInputProfile(activeProfile);
    aInputManager.SetActiveProfile(0);
    aResourceManager.UpdateCamera(aRenderer.GetAspect());

    auto prevTime = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> curTime;

    std::vector<MeshInfo> meshInfos;
    const char cTitle[] = "FPS:";
    AnA::String title{cTitle, sizeof(cTitle), sizeof(cTitle) + 40};

    auto window = aWindow.GetGLFWwindow();
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        curTime = std::chrono::high_resolution_clock::now();
        frameTime = std::chrono::duration<float, std::chrono::seconds::period>(curTime - prevTime).count();
        prevTime = curTime;
        prevSecond += frameTime;
        frameCount++;

        if (prevSecond >= 1.0f)
        {
            fps = frameCount;
            std::string terrainSize = std::to_string(int(terrainPushConstants.density * 320.0f * 32.0f));
            auto info = std::to_string(static_cast<int>(frameCount.As<float>() / prevSecond)) +
                " CPU Time: " + std::to_string(cpuTimeBeforeRecord * 1000.0f) + "ms GPU Time:" +
                std::to_string(aRenderer.GetGPUTime()) + "ms Command Time:" +
                std::to_string((cpuTime - cpuTimeBeforeRecord) * 1000.0f) + "ms Terrain Size:" +
                terrainSize + "x" + terrainSize + " Window Size:" +
                std::to_string(aWindow.Width) + "x" + std::to_string(aWindow.Height);
            title.Copy(info.c_str(), info.length(), sizeof(cTitle) - 1);
            //aResourceManager.TextContext.UpdateText(0, title.Str());
            glfwSetWindowTitle(aWindow.GetGLFWwindow(), title.Str());
            //printf("{\"FPS\":%d}\r", frameCount.value);
            prevSecond = 0.0f;
            frameCount = 0;
        }
        camera.SetSpeedRatio(frameTime * 2.0f);
        aInputManager.Check();
        //Update Resources
        commandBufferNeedUpdate = aResourceManager.MainScene.BeginCommandBufferUpdate() || aRenderer.NeedUpdate();

        if (aRenderer.NeedUpdate())
        {
            aResourceManager.Resize();
            updateSceneOffset();
        }
        if ((aResourceManager.MainControl &&
            aResourceManager.MainControl->NeedUpdate()) || aRenderer.NeedUpdate())
        {
            auto controlExtent = aRenderer.GetSwapChainExtent();
            controlExtent.width = static_cast<uint32_t>(actualSceneOffset.x);
            aResourceManager.MainControl->Aspect = static_cast<float>(controlExtent.width) / static_cast<float>(controlExtent.height);
            aResourceManager.MainControl->Extent = controlExtent;
            aResourceManager.Shapes.Extent = controlExtent;
            if (aDevice.MeshShaderSupport())
            {
                if (Controls::Control::TextLayoutNeedReset())
                {
                    aResourceManager.TextContext.ResetLayout();
                    Controls::Control::EndTextLayoutReset();
                }
            }
            aResourceManager.Shapes.PrepareDraw(aResourceManager.MainControl);
            aResourceManager.MainControl->EndUpdate();
        }
        aResourceManager.Update();
        if (loopCallback)
            loopCallback();

        //Record Primary Command Buffer
        cpuTimeBeforeRecord = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - prevTime).count();

        if (auto commandBuffer = aRenderer.BeginFrame())
        {
            onCommandBufferRecording(*commandBuffer);
            aRenderer.EndFrame();
        }
        cpuTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - prevTime).count();
    }
    vkDeviceWaitIdle(aDevice.GetLogicalDevice());
}

void App::Exit()
{
    glfwSetWindowShouldClose(aWindow.GetGLFWwindow(), 1);
}

void App::Cleanup()
{
    _2DModel.reset();
}

std::shared_ptr<Model> &App::Get2DModel()
{
    if (_2DModel == nullptr)
    {
        std::vector<Model::Vertex> vertices
        {
            {{-1.0f, -1.0f, 0.f}, {}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, 0.f}, {}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 0.f}, {}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, 0.f}, {}, {1.0f, 1.0f}}
        };
        Model::ModelInfo modelInfo{{}, vertices, {}, {}, {}, {0, 2, 1, 1, 2, 3}};
        _2DModel = std::make_shared<Model>(modelInfo);
    }

    return _2DModel;
}

void App::CreateModel(const Model::ModelInfo &modelInfo, std::shared_ptr<Model> &model)
{
    model = std::make_shared<Model>(modelInfo);
}

void App::startUILoop(std::thread &loopThread)
{
    _uiSignal = UI_SIGNAL_WAIT;
    loopThread = std::thread(App::uiLoop);
    loopThread.detach();
}

void App::waitUILoop(std::thread &loopThread)
{
    _uiLoopShouldEnd = true;
    if (loopThread.joinable())
        loopThread.join();
}

void App::uiLoop()
{
    while (!_uiLoopShouldEnd)
    {
        switch (_uiSignal)
        {
        case UI_SIGNAL_KEY:
            {/*
            auto &camera = _aApp->GetCamera();
            auto &key = _uiParam[0];
            if (key == GLFW_KEY_W)
                camera.MoveForward();
            if (key == GLFW_KEY_S)
                camera.MoveBack();
            if (key == GLFW_KEY_A)
                camera.MoveLeft();
            if (key == GLFW_KEY_D)
                camera.MoveRight();
            if (key == GLFW_KEY_SPACE)
                camera.MoveUp();
            if (key == GLFW_KEY_C)
                camera.MoveDown();*/
            }
            _uiSignal = UI_SIGNAL_WAIT;
            break;
        case UI_SIGNAL_EXIT:
            return;
        default:
            break;
        }
    }
}

void App::onCommandBufferRecording(CommandBuffer& commandBuffer)
{
    auto& swapChain = aRenderer.GetSwapChain();

#ifdef DEFERRED
    aRenderer.BeginOffscreenRendering(commandBuffer);

    swapChain.SetViewport(commandBuffer);
    aRenderer.RenderIndirect(commandBuffer, aResourceManager.MainScene,
        aResourceManager.Shaders[5],
        swapChain.CurrentFrame);

    aRenderer.EndOffscreenRendering(commandBuffer);
#endif
#ifndef RELEASE_BUILD
    Device::StageBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    auto& computeShader = aResourceManager.Shaders[7];
    computeShader.GetPipeline().Bind(commandBuffer);
    auto& computeSets = computeShader.GetDescriptorSets()[aResourceManager.MainScene.GetBufferIndex()];
    computeSets[0] = aResourceManager.MainScene.GetVertexDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        computeShader.GetPipelineLayout(), 0, 1,
        computeSets.data(), 0, VK_NULL_HANDLE);
    vkCmdDispatch(commandBuffer, (aResourceManager.MainScene.GetMeshCount() + 63) / 64, 1, 1);
    Device::StageBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
#endif
    aRenderer.BeginRendering(commandBuffer);
    swapChain.SetViewport(commandBuffer, actualSceneOffset);

#ifdef DEFERRED
    auto& lightShader = aResourceManager.Shaders.back();
    lightShader.GetPipeline().Bind(commandBuffer);
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    auto& sets = lightShader.GetDescriptorSets()[0];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightShader.GetPipelineLayout(),
        0, uint32_t(sets.size()), sets.data(), 0, VK_NULL_HANDLE);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
#else
    aRenderer.RenderIndirect(commandBuffer, aResourceManager.MainScene,
        aDevice.MeshShaderSupport() ? aResourceManager.Shaders[5] : aResourceManager.Shaders[0],
        swapChain.CurrentFrame);
#endif

    if (aDevice.MeshShaderSupport())
    {
        swapChain.SetViewport(commandBuffer);
        aRenderer.RenderIndirect(commandBuffer, aResourceManager.TextContext,
            aResourceManager.Shaders[6], swapChain.CurrentFrame);
    }
    swapChain.SetViewport(commandBuffer, aResourceManager.Shapes.Extent);
    aRenderer.RenderIndirect(commandBuffer, aResourceManager.Shapes,
        aResourceManager.Shaders[1], swapChain.CurrentFrame);
    //Device.StageBarrier(commandBuffer,
    //    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT|VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
    aRenderer.EndRendering(commandBuffer);
}
