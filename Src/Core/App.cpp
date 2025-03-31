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
    aWindow = new Window();
    if (aWindow->Init())
        throw std::runtime_error("Failed to init window!");
    aInputManager = new Input::InputManager(*aWindow);

    //glfwSetKeyCallback(aWindow->GetGLFWwindow(), App::keyCallback);
    aInstance = new Instance;
    aWindow->CreateWindowSurface(aInstance);
    _aDevice= aDevice = new Device(aInstance->GetInstance(), aWindow->GetSurface());
    aRenderer = new Renderer(*aWindow, aDevice);
    aResourceManager = new Resource::ResourceManager(aDevice);
    aRenderSystem = new Systems::RenderSystem(aDevice, aRenderer->GetSwapChain());
    aShadowSystem = new Systems::ShadowSystem(aDevice, &aRenderer->GetSwapChain());
    createRecordCallBacks();
}

void App::Run()
{
    Cameras::Camera& camera = aResourceManager->MainCamera;
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
        meshInfos.push_back({"Models/cube.obj", {{random_double(), random_double() - 2.0f, random_double() + 5.0f}, {random_double(), random_double(), random_double()}}, (uint32_t)rand() % 4});
    const char cTitle[] = "AnA FPS:";
    AnA::String title{cTitle, sizeof(cTitle), sizeof(cTitle) + 10};
    int frameCount = 0;
    float prevSecond = 0.0f;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        //Test Mesh Appending at runtime
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
        prevSecond += frameTime;
        frameCount++;
        if (prevSecond >= 1.0f)
        {
            //Show FPS on title bar
            auto fps = std::to_string((int)(frameCount / prevSecond));
            title.Copy(fps.c_str(), fps.length() + 1, sizeof(cTitle) - 1);
            glfwSetWindowTitle(aWindow->GetGLFWwindow(), title.Str());
            prevSecond = 0.0f;
            frameCount = 0;
        }
        camera.SetSpeedRatio(frameTime * 2.0);
        aInputManager->Check();
        //Update Resources
        _aApp->commandBufferNeedUpdate = aResourceManager->SceneObjects.BeginCommandBufferUpdate() || aRenderer->NeedUpdate();
        if (aRenderer->NeedUpdate())
        {
            aResourceManager->Resize();
        }
        aResourceManager->Update();
        onLoop();
        //Record Primary Command Buffer
        if (auto commandBuffer = aRenderer->BeginFrame())
        {
            onCommandBufferRecording(commandBuffer);
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

void renderMeshesIndirecct(VkCommandBuffer commandBuffer)
{
    auto aResourceManager = Resource::ResourceManager::GetCurrent();
    Systems::RenderSystem::GetCurrent()->RenderMeshes(commandBuffer, 
        aResourceManager->SceneObjects, 
        aResourceManager->Shaders.back());
    /*
    Systems::RenderSystem::GetCurrent()->RenderMeshesIndirect(commandBuffer, 
        aResourceManager->SceneObjects, 
        aResourceManager->Shaders[0]);
        */
}

void RenderShapesIndirect(VkCommandBuffer commandBuffer)
{
    auto aResourceManager = Resource::ResourceManager::GetCurrent();
    auto aRenderSystem = Systems::RenderSystem::GetCurrent();
    aRenderSystem->RenderShapesIndirect(commandBuffer, 
        aResourceManager->Shapes, 
        aResourceManager->Shaders[1]);
}

void App::createRecordCallBacks()
{
    auto& RecordCallBacks = aResourceManager->RecordCallBacks;
    RecordCallBacks.emplace_back([]() 
    {
        return _aApp->commandBufferNeedUpdate;
    }, [](VkOffset2D& offset, VkExtent2D& extent)
    {
        auto aResourceManager = Resource::ResourceManager::GetCurrent();
        auto& aRenderer = _aApp->GetRenderer();
        aResourceManager->SecondaryCommandBufferPool.Reset();
        
        aResourceManager->SecondaryCommandBufferPool.Enqueue([](VkCommandBuffer secondaryCommandBuffer, size_t index)
        {
            renderMeshesIndirecct(secondaryCommandBuffer);
        }, &aRenderer.GetInheritanceInfo(RENDER_PASS_TYPE_ONSCREEN), _aApp->GetSceneOffset());
        /*
        aRenderer.RecordOffscreenSecondaryCommandBuffer([](VkCommandBuffer offScreenSecondaryCommandBuffer)
        {
            //RenderShadowsIndirect(offScreenSecondaryCommandBuffer);
        });*/
        aResourceManager->SceneObjects.EndCommandBufferUpdate();
    });
#ifdef ANA_INCLUDE_CONTROL

    RecordCallBacks.emplace_back([]()
    {
        auto aResourceManager = Resource::ResourceManager::GetCurrent();
        return aResourceManager->MainControl != nullptr && _aApp->commandBufferNeedUpdate;
    }, [](VkOffset2D& offset, VkExtent2D& extent)
    {
        //record controls here
        auto aResourceManager = Resource::ResourceManager::GetCurrent();
        auto& aRenderer = _aApp->GetRenderer();
        auto controlExtent = aRenderer.GetSwapChainExtent();
        controlExtent.width = _aApp->GetSceneOffset().x;
        aResourceManager->MainControl->Aspect = (float)controlExtent.width / (float)controlExtent.height;
        aResourceManager->MainControl->Extent = controlExtent;
        auto& shapes = aResourceManager->Shapes;
        shapes.Offset = offset;
        shapes.Extent = controlExtent;
        shapes.PrepareDraw(aResourceManager->MainControl);
        aResourceManager->SecondaryCommandBufferPool.Enqueue([](VkCommandBuffer secondaryCommandBuffer, size_t index)
        {
            RenderShapesIndirect(secondaryCommandBuffer);
        }, &aRenderer.GetInheritanceInfo(RENDER_PASS_TYPE_ONSCREEN), offset, controlExtent);
    });
#endif
}

void App::onCommandBufferRecording(VkCommandBuffer& commandBuffer)
{
    if (!aResourceManager->SecondaryCommandBufferPool.GetCommandBufferCount() || !aResourceManager->SceneObjects.GetMeshCount())
        return;
    //if (commandBufferNeedUpdate)
    //{
    /*
    uint32_t frameIndex = aRenderer->GetFrameIndex();
    for (uint32_t i = 0 ; i < SHADOW_MAP_CASCADE_COUNT; i++)
    {
        aRenderer->BeginOffscreenRenderPass(commandBuffer, 
            aResourceManager->ShadowMap.GetCascades()[i].framebuffers[frameIndex],
            VK_SUBPASS_CONTENTS_INLINE);
        
        //aRenderer->ExecuteOffscreenSecondaryCommandBuffer(commandBuffer);
        aShadowSystem->RenderCascadedShadowsIndirect(commandBuffer, aResourceManager->SceneObjects, aResourceManager->Shaders[2], i);
        aRenderer->EndRenderPass(commandBuffer);
    }*/
    //}
    aRenderer->BeginSwapChainRenderPass(commandBuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    aResourceManager->SecondaryCommandBufferPool.ExecuteRecordedBuffer(commandBuffer);
    //aRenderSystem->RenderMeshesIndirect(commandBuffer, aResourceManager->SceneObjects, aResourceManager->Shaders[0]);
    //aRenderSystem->RenderShapesIndirect(commandBuffer, *aResourceManager->Shapes, aResourceManager->Shaders[1]);
    aRenderer->EndRenderPass(commandBuffer);
}

void App::onLoop()
{
    
}