#include "Headers/App.hpp" 
#include "Camera/Headers/CameraController.hpp"
#include "Input/Headers/InputManager.hpp"
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

App::App() : aWindow(),
    aInstance(aWindow),
    aInputManager(aWindow),
    aDevice(aInstance.GetInstance(), aWindow.GetSurface()),
    aRenderer(aWindow, &aDevice),
    aRenderSystem(),
    aShadowSystem(&aRenderer.GetSwapChain()),
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

    Model::ModelInfo modelInfo = {vertices, {}, {}, 4, {0, 1, 2, 1, 2, 3}};
    model = std::make_shared<Model>(_aDevice, modelInfo);
}

void App::Init()
{
    //glfwSetKeyCallback(aWindow->GetGLFWwindow(), App::keyCallback);
    createRecordCallBacks();
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
    Float frameTime, cpuTime, prevSecond;
    Int32 frameCount = 0;

    std::vector<MeshInfo> meshInfos;
    const char cTitle[] = "AnA FPS:";
    AnA::String title{cTitle, sizeof(cTitle), sizeof(cTitle) + 20};

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
            auto info = std::to_string(static_cast<int>(frameCount.As<float>() / prevSecond)) +
                " CPU Time: " + std::to_string(cpuTime * 1000.0f) + "ms GPU Time:" + 
                std::to_string(aRenderer.GetGPUTime()) + "ms";
            title.Copy(info.c_str(), info.length(), sizeof(cTitle) - 1);
            glfwSetWindowTitle(aWindow.GetGLFWwindow(), title.Str());
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
        }
        aResourceManager.Update();
        onLoop();

        //Record Primary Command Buffer
        if (auto commandBuffer = aRenderer.BeginFrame())
        {
            onCommandBufferRecording(*commandBuffer);
            aRenderer.EndFrame();
        }
        cpuTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - prevTime).count();
    }
    vkDeviceWaitIdle(aDevice.GetLogicalDevice());
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
        Model::ModelInfo modelInfo{vertices, {}, {}, 4, {0, 2, 1, 1, 2, 3}};
        _2DModel = std::make_shared<Model>(_aDevice, modelInfo);
    }

    return _2DModel;
}

void App::CreateModel(const Model::ModelInfo &modelInfo, std::shared_ptr<Model> &model)
{
    model = std::make_shared<Model>(_aDevice, modelInfo);
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

void App::createRecordCallBacks()
{
    auto& RecordCallBacks = aResourceManager.RecordCallBacks;
    RecordCallBacks.emplace_back([]() 
    {
        return _aApp->commandBufferNeedUpdate;
    }, [](VkOffset2D& , VkExtent2D& )
    {
        auto aResourceManager = Resource::ResourceManager::GetCurrent();
        auto& aRenderer = _aApp->GetRenderer();        
        aResourceManager->SecondaryCommandBufferPool.Enqueue([](CommandBuffer& secondaryCommandBuffer, size_t )
        {
            auto aResourceManager = Resource::ResourceManager::GetCurrent();
            Systems::RenderSystem::GetCurrent()->RenderIndirect(secondaryCommandBuffer, 
                aResourceManager->MainScene, 
                aResourceManager->SecondaryCommandBufferPool.CurrentBufferIndex);
        }, &aRenderer.GetInheritanceInfo(RENDER_PASS_TYPE_ONSCREEN), _aApp->GetSceneOffset());
        /*
        aRenderer.RecordOffscreenSecondaryCommandBuffer([](CommandBuffer& offScreenSecondaryCommandBuffer)
        {
            //RenderShadowsIndirect(offScreenSecondaryCommandBuffer);
        });*/
        aResourceManager->MainScene.EndCommandBufferUpdate();
    }, GetSceneOffset());
#ifdef ANA_INCLUDE_CONTROL

    RecordCallBacks.emplace_back([]()
    {
        auto aResourceManager = Resource::ResourceManager::GetCurrent();
        return aResourceManager->MainControl != nullptr && _aApp->commandBufferNeedUpdate;
    }, [](VkOffset2D& offset, VkExtent2D& )
    {
        //record controls here
        auto aResourceManager = Resource::ResourceManager::GetCurrent();
        auto& aRenderer = _aApp->GetRenderer();
        auto controlExtent = aRenderer.GetSwapChainExtent();
        controlExtent.width = static_cast<uint32_t>(_aApp->GetSceneOffset().x);
        aResourceManager->MainControl->Aspect = static_cast<float>(controlExtent.width) / static_cast<float>(controlExtent.height);
        aResourceManager->MainControl->Extent = controlExtent;
        auto& shapes = aResourceManager->Shapes;
        shapes.Offset = offset;
        shapes.Extent = controlExtent;
        shapes.PrepareDraw(aResourceManager->MainControl);
        aResourceManager->SecondaryCommandBufferPool.Enqueue([](CommandBuffer& secondaryCommandBuffer, size_t )
        {
            auto aResourceManager = Resource::ResourceManager::GetCurrent();
            auto aRenderSystem = Systems::RenderSystem::GetCurrent();
            aRenderSystem->RenderIndirect(secondaryCommandBuffer, 
                aResourceManager->Shapes);
        }, &aRenderer.GetInheritanceInfo(RENDER_PASS_TYPE_ONSCREEN), offset, controlExtent);
    });
#endif
}

void App::onCommandBufferRecording(CommandBuffer& commandBuffer)
{
    //if (commandBufferNeedUpdate)
    //{
    
    uint32_t frameIndex = aRenderer.GetFrameIndex();
    for (uint32_t i = 0 ; i < SHADOW_MAP_CASCADE_COUNT; i++)
    {
        aRenderer.BeginOffscreenRenderPass(commandBuffer, 
            aResourceManager.ShadowMap.GetCascades()[i].framebuffers[frameIndex],
            VK_SUBPASS_CONTENTS_INLINE);
        
        aShadowSystem.RenderCascadedShadowsIndirect(commandBuffer, aResourceManager.MainScene, aResourceManager.Shaders[2], i);
        aRenderer.EndRenderPass(commandBuffer);
    }
    aRenderer.BeginSwapChainRenderPass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR);
    if (aResourceManager.SecondaryCommandBufferPool.GetCommandBufferCount())
        aResourceManager.SecondaryCommandBufferPool.ExecuteRecordedBuffer(commandBuffer);
    aRenderer.EndRenderPass(commandBuffer);
}

void App::onLoop()
{
    
}