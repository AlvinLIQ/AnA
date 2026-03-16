#pragma once

#include "../../Headers/Device.hpp"
#include "../../Headers/Buffer.hpp"
#include "Model.hpp"

#include <array>
#include <set>
#include <memory>
#include <unordered_map>

namespace AnA
{
    namespace Resources
    {
        class Meshes
        {
        public:
            struct MeshletInfo
            {
                uint32_t vertexOffset;
                uint32_t indexOffset;
                uint32_t vertexCount;
                uint32_t indexCount;
            };
            struct BoundingSphere
            {
                glm::vec3 center;
                float radius;
                glm::vec3 normal;
                float cutoff;
                glm::vec3 coneApex;
                float padding;
            };

            struct MeshFrameResource
            {
                Buffer vertexBuffer;
                Buffer indexBuffer;
                //Buffer objectBuffer;
                Buffer meshletBuffer;
                Buffer meshletVertexBuffer;
                Buffer meshletIndexBuffer;
                Buffer meshletCullingBuffer;
                //Buffer meshletIDBuffer;
                Buffer meshletIDCountBuffer;
            };

            Meshes(Device* mDevice);
            ~Meshes();

            bool Create(const char* filePath, uint32_t& id);
            void Load(const char* filePath);
            void Load(const uint32_t id);

            std::unordered_map<uint32_t, std::shared_ptr<Model>> MeshMap{};
            std::unordered_map<std::string, uint32_t> MeshPathIndexMap{};
        private:
            Device* aDevice;
            std::set<uint32_t> loadedSet{};

            uint32_t currentBufferIndex = 0;

            std::array<MeshFrameResource, MAX_FRAMES_IN_FLIGHT> frameResources;
            void initFrameResource(MeshFrameResource& frameResource);
            void prepareFrameResources();

            size_t vertexCount = 0;
            size_t indexCount = 0;
            uint32_t meshletVertexCount = 0;
            uint32_t meshletIndexCount = 0;
            uint32_t meshletCount = 0;
            uint32_t meshletIDCount = 0;
        };
    }
}
