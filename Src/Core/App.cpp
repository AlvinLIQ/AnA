#include "Headers/App.hpp"
#include "Camera/Headers/CameraController.hpp"
#include "Resources/Headers/ResourceManager.hpp"
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <chrono>

using namespace AnA;

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

void App::Init()
{
    //glfwSetKeyCallback(aWindow->GetGLFWwindow(), App::keyCallback);
    updateSceneOffset();
}

void App::Run()
{
    Cameras::Camera& camera = aResourceManager.MainCamera;
    Cameras::CameraController cameraController{camera};
    auto& firstProfile = aInputManager.GetProfiles()[0];
    cameraController.GetInputProfile(firstProfile);
    aInputManager.SetActiveProfile(aInputManager.GetActiveProfileIndex());
    aResourceManager.UpdateCamera(aRenderer.GetAspect());

    auto prevTime = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> curTime;

    std::vector<MeshInfo> meshInfos;
    const char cTitle[] = "FPS:";
    AnA::String title{cTitle, sizeof(cTitle), sizeof(cTitle) + 40};

    while(!aWindow.GetExitSignal())
    {
        aWindow.PollEvents();
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
            aWindow.SetTitle(title.Str());
            //glfwSetWindowTitle(aWindow.GetGLFWwindow(), title.Str());
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
#ifdef ANA_INCLUDE_CONTROL
        if (aResourceManager.MainControl &&
            (aResourceManager.MainControl->NeedUpdate() || aRenderer.NeedUpdate()))
        {
            auto controlExtent = aRenderer.GetSwapChainExtent();
            if (actualSceneOffset.x)
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
#endif

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
    aWindow.Exit();
}

void App::Cleanup()
{
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
            if (key == SDL_SCANCODE_W)
                camera.MoveForward();
            if (key == SDL_SCANCODE_S)
                camera.MoveBack();
            if (key == SDL_SCANCODE_A)
                camera.MoveLeft();
            if (key == SDL_SCANCODE_D)
                camera.MoveRight();
            if (key == SDL_SCANCODE_SPACE)
                camera.MoveUp();
            if (key == SDL_SCANCODE_C)
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

    aRenderer.BeginRendering(commandBuffer);

    swapChain.SetViewport(commandBuffer, actualSceneOffset);
    aRenderer.RenderIndirect(commandBuffer, aResourceManager.MainScene,
        aResourceManager.Shaders[0]);

    swapChain.SetViewport(commandBuffer);
    aRenderer.RenderIndirect(commandBuffer, aResourceManager.TextContext,
        aResourceManager.Shaders[1]);

    swapChain.SetViewport(commandBuffer, aResourceManager.Shapes.Extent);
    aRenderer.RenderIndirect(commandBuffer, aResourceManager.Shapes,
        aResourceManager.Shaders[2]);

    aRenderer.EndRendering(commandBuffer);
}
