#pragma once

#include "../../Headers/Device.hpp"
#include "../../Headers/Buffer.hpp"
#include "Mesh.hpp"

#include <array>
#include <set>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace AnA
{
    namespace Resources
    {
        class Meshes
        {
        public:
            struct MeshFrameResource
            {
                Buffer meshletBuffer;
                Buffer meshBuffer;
            };

            Meshes(Device* mDevice);
            ~Meshes();

            void Init();

            bool Create(const char* filePath, uint32_t& id, uint32_t& count);
            bool Create(std::shared_ptr<Mesh> model, uint32_t& id);
            void Load(const char* filePath, uint32_t& id, uint32_t& count);
            void Load(std::shared_ptr<Mesh> model, uint32_t& id);
            void Load(const uint32_t id);

            void Append(uint32_t id, std::vector<AnA::Mesh::Vertex>& vertices);

            bool NeedUpdate()
            {
                return needUpdate;
            }
            void Update();
            void UpdateMesh(uint32_t id);

            MeshFrameResource& GetCurrentFrameResource()
            {
                return frameResources[currentBufferIndex];
            }

            uint32_t GetVertexCount()
            {
                return uint32_t(vertexCount);
            }
            uint32_t GetMeshletCount()
            {
                return meshletCount;
            }

            std::unordered_map<uint32_t, std::shared_ptr<Mesh>> MeshMap{};
            std::unordered_map<std::string, uint32_t> MeshPathIndexMap{};
        private:
            Device* aDevice;
            std::set<uint32_t> loadedSet{};

            std::mutex updateMutex;
            uint32_t currentBufferIndex = 0;
            bool needUpdate = false;
            std::mutex _mutex;

            std::array<MeshFrameResource, MAX_FRAMES_IN_FLIGHT> frameResources;
            void initFrameResource(MeshFrameResource& frameResource);
            void rebuildFrameResource(MeshFrameResource& frameResource);
            uint32_t prepareFrameResources();

            size_t vertexCount = 0;
            size_t indexCount = 0;
            uint32_t meshletCount = 0;
        };
    }
}
