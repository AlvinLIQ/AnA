#pragma once

#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "../Input/Headers/InputManager.hpp"
#include "../Resources/Headers/ResourceManager.hpp"
#include <thread>

namespace AnA
{
    class App
    {
    public:
        App(const char* name = "AnA");
        virtual ~App();

        App(const App&) = delete;
        App &operator=(const App&) = delete;

        void Init();
        void Run();
        void Cleanup();
        void Exit();

        static std::shared_ptr<Mesh> &Get2DModel();
        static void CreateModel(const Mesh::MeshData &meshData, std::shared_ptr<Mesh> &model);

        static App* GetCurrent();
        Input::InputManager& GetInputManager()
        {
            return aInputManager;
        }

        SwapChain& GetSwapChain()
        {
            return aRenderer.GetSwapChain();
        }
        Renderer& GetRenderer()
        {
            return aRenderer;
        }
        Window& GetWindow()
        {
            return aWindow;
        }

        Device* GetDevice()
        {
            return &aDevice;
        }
    private:
        std::thread uiThread;
        static void startUILoop(std::thread &loopThread);
        static void waitUILoop(std::thread &loopThread);
        static void uiLoop();
        //static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    protected:
        virtual void onCommandBufferRecording(CommandBuffer& commandBuffer);
        Window aWindow;
        Instance aInstance;
        Input::InputManager aInputManager;
        Device aDevice;
        Renderer aRenderer;
        Resources::ResourceManager aResourceManager;
        VkOffset2D sceneOffset{};
        Float frameTime, cpuTime, cpuTimeBeforeRecord, prevSecond;
        Int32 frameCount = 0, fps = 0;
        bool commandBufferNeedUpdate = false;
        TerrainPushConstants terrainPushConstants = {0.5f, 5.0f, 0, 0};
        void(*loopCallback)() = nullptr;
    };
}
