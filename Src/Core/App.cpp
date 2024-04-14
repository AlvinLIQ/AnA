#include "Headers/App.hpp" 
#include "Camera/Headers/CameraController.hpp"
#include "Input/Headers/InputManager.hpp"
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <memory>

using namespace AnA;

std::shared_ptr<Model> _2DModel;
Device* _aDevice;
App* _aApp;

#define UI_SIGNAL_EXIT 0
#define UI_SIGNAL_KEY 2
#define UI_SIGNAL_MOUSE 4
#define UI_SIGNAL_WAIT 1

bool _uiLoopShouldEnd = false;
short _uiSignal;
int _uiParam[2];

App::App()
{
    _aApp = this;
}

App::~App()
{
    Cleanup();
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
    model = std::make_shared<Model>(*_aDevice, modelInfo);
}

void App::Init()
{
    aWindow = new Window();
    if (aWindow->Init())
        throw std::runtime_error("Failed to init window!");
    aInputManager = new Input::InputManager(*aWindow);

    //glfwSetKeyCallback(aWindow->GetGLFWwindow(), App::keyCallback);
    aInstance = new Instance;
    aWindow->CreateWindowSurface(aInstance);
    _aDevice = aDevice = new Device(aInstance->GetInstance(), aWindow->GetSurface());
    aRenderer = new Renderer(*aWindow, *aDevice);
    aResourceManager = new Resource::ResourceManager(*aDevice);
    aRenderSystem = new Systems::RenderSystem(*aDevice, aRenderer->GetSwapChain());
    aShadowSystem = new Systems::ShadowSystem(*aDevice, &aRenderer->GetSwapChain());
}

void App::Run(RecordCallBackEx recordCallBack)
{
    if (recordCallBack == nullptr)
        recordCallBack = [](VkCommandBuffer secondaryCommandBuffer, size_t index)
            {
                auto resourceManager = Resource::ResourceManager::GetCurrent();
                Systems::RenderSystem::GetCurrent()->RenderBatch(secondaryCommandBuffer, 
                    resourceManager->SceneObjects, 
                    *resourceManager->Shaders[0], index);
            };
    auto offscreenRecordCallBack = [](VkCommandBuffer secondaryCommandBuffer)
            {
                auto aResourceManager = Resource::ResourceManager::GetCurrent();
                auto aShadowSystem = Systems::ShadowSystem::GetCurrent();
                aShadowSystem->RenderShadows(secondaryCommandBuffer, aResourceManager->SceneObjects, *aResourceManager->Shaders[2]);
            };

    Cameras::Camera& camera = aResourceManager->MainCamera, &lightCamera = aResourceManager->LightCamera;
    Cameras::CameraController cameraController{camera};
    auto& activeProfile = aInputManager->GetActiveProfile();
    cameraController.GetInputProfile(activeProfile);
    aInputManager->SetActiveProfile(0);
    //startUILoop(uiThread);
    auto window = aWindow->GetGLFWwindow();
    //camera.SetViewDirection({}, glm::vec3(0.5f, 0.f, 1.f));
    //camera.SetViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
    auto prevTime = std::chrono::high_resolution_clock::now();
    aResourceManager->UpdateCamera(aRenderer->GetAspect());
    std::vector<MeshInfo> meshInfos;
    bool pressed = false;
    for (int i = 0; i < 1000; i++)
        meshInfos.push_back({"Models/cube.obj", {{drand48(), drand48(), drand48()}, {drand48(), drand48(), drand48()}}});
    bool commandBufferNeedUpdate;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (!pressed && glfwGetKey(aWindow->GetGLFWwindow(), GLFW_KEY_F) == GLFW_PRESS)
        {
            pressed = true;
            aResourceManager->TaskPool.Enqueue([this, &meshInfos]()
            {
                aResourceManager->SceneObjects.Append(meshInfos);
            });
        }
        auto curTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(curTime - prevTime).count();
        prevTime = curTime;
        printf("FPS:%.2f\r", 1.0f / frameTime);
        camera.SetSpeedRatio(frameTime);
        aInputManager->CheckAndRunCallbacks();
        //Update Resources
        if (aRenderer->NeedUpdate())
        {
            aResourceManager->Resize();
        }
        aResourceManager->Update();
        if ((commandBufferNeedUpdate = (aResourceManager->SceneObjects.BeginCommandBufferUpdate() || aRenderer->NeedUpdate())))
        {
            aResourceManager->SecondaryCommandBufferPool.Reset();
            aResourceManager->SecondaryCommandBufferPool.Enqueue(recordCallBack, 
                &aRenderer->GetInheritanceInfo(RENDER_PASS_TYPE_ONSCREEN));
            aRenderer->RecordSecondaryCommandBuffer(offscreenRecordCallBack, RENDER_PASS_TYPE_OFFSCREEN);
            aResourceManager->SceneObjects.EndCommandBufferUpdate();
        }
        //Record Primary Command Buffer
        if (auto commandBuffer = aRenderer->BeginFrame())
        {
            if (commandBufferNeedUpdate)
            {
                aRenderer->BeginOffscreenRenderPass(commandBuffer, 
                    aResourceManager->GetShadowFramebuffers()[aResourceManager->SecondaryCommandBufferPool.CurrentBufferIndex]);
                aRenderer->ExcuteSecondaryCommandBuffer(commandBuffer, RENDER_PASS_TYPE_OFFSCREEN);
                aRenderer->EndRenderPass(commandBuffer);
            }
            while(!aResourceManager->SecondaryCommandBufferPool); //Sync
            aRenderer->BeginSwapChainRenderPass(commandBuffer);
            aResourceManager->SecondaryCommandBufferPool.ExcuteRecordedBuffer(commandBuffer);
            aRenderer->EndRenderPass(commandBuffer);
            aRenderer->EndFrame();
        }
    }
    //waitUILoop(uiThread);
    vkDeviceWaitIdle(aDevice->GetLogicalDevice());
}

void App::Cleanup()
{
    delete aInputManager;
    delete aRenderer;
    delete aRenderSystem;
    delete aShadowSystem;

    _2DModel.reset();
    delete aResourceManager;

    delete aDevice;
    vkDestroySurfaceKHR(aInstance->GetInstance(), aWindow->GetSurface(), nullptr);
    delete aInstance;
    delete aWindow;
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
        Model::ModelInfo modelInfo{vertices, {}, {}, 4, {0, 1, 2, 1, 2, 3}};
        _2DModel = std::make_shared<Model>(*_aDevice, modelInfo);
    }

    return _2DModel;
}

void App::CreateModel(const Model::ModelInfo &modelInfo, std::shared_ptr<Model> &model)
{
    model = std::make_shared<Model>(*_aDevice, modelInfo);
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
        }
    }
}

void App::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    /*
    _uiParam[0] = key;
    _uiParam[1] = action;
    _uiSignal = UI_SIGNAL_KEY;
   */
    //glfwGetKey
}