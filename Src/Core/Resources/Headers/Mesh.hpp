#pragma once

#include <volk.h>
#include "../../Headers/Buffer.hpp"
#include "Animation.hpp"

#include <functional>
#include <glm/fwd.hpp>
#include <glm/gtc/epsilon.hpp>

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>
#include <string>

namespace AnA
{
    struct Mesh
    {
        struct Vertex
        {
            glm::vec3 position{};
            uint16_t pitch{};
            uint16_t yaw{};
            uint32_t textureId;
            glm::u8vec3 color{};
            glm::vec2 uv{};

            bool operator==(const Vertex& vertex) const
            {
                return position == vertex.position && pitch == vertex.pitch && yaw == vertex.yaw && uv == vertex.uv;
            }
            static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
        };
        struct VertexHash
        {
            std::size_t operator() (const Vertex& vertex) const
            {
                return std::hash<float>()(vertex.position.x) ^ std::hash<float>()(vertex.position.y) ^ std::hash<float>()(vertex.position.z)
                    ^ std::hash<float>()(vertex.pitch)
                    ^ std::hash<float>()(vertex.yaw)
                    ^ std::hash<float>()(vertex.uv.x)
                    ^ std::hash<float>()(vertex.uv.y);
            }
        };
        struct VectorHash
        {
            std::size_t operator() (const glm::vec3& vector) const
            {
                return std::hash<float>()(vector.x) ^
                    std::hash<float>()(vector.y) << 1 ^
                    std::hash<float>()(vector.z) << 2;
            }
        };
        struct VectorEqual
        {
            bool operator()(const glm::vec3& a, const glm::vec3& b) const noexcept
            {
                return glm::all(glm::epsilonEqual(a, b, 1e-5f));
            }
        };
        struct Vec3Less
        {
            bool operator()(const glm::vec3& a, const glm::vec3& b) const
            {
                if (a.x != b.x) return a.x < b.x;
                if (a.y != b.y) return a.y < b.y;
                return a.z < b.z;
            }
        };
        struct Meshlet
        {
            uint32_t vertices[64];
            uint8_t indices[128 * 3];
            uint32_t indexCount;
            uint32_t vertexCount;
            float radius;
            glm::vec3 center;
            glm::vec3 normal;
            glm::vec3 coneApex;
            float cutoff;
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
        struct MeshletInfo
        {
            uint32_t vertexOffset;
            uint32_t indexOffset;
            uint32_t vertexCount;
            uint32_t indexCount;
            BoundingSphere bounding;
        };
        struct BSPNode
        {
            BSPNode* left = nullptr;
            BSPNode* right = nullptr;
            std::vector<Vertex>* vertices = nullptr;
        };
        typedef uint32_t Index;

        struct Node
        {
            std::vector<Index> indices{};
            glm::mat4 transform{};
        };

        struct MeshData
        {
            std::vector<Node> nodes;
            std::vector<Vertex> vertices;
            glm::vec3 minBounding;
            glm::vec3 maxBounding;
            Index indexStep;
            std::vector<Index> indices;
            std::string texturePath;
        };

        Mesh(const MeshData& meshData);
        ~Mesh();

        MeshData data{};
        glm::vec3 center{};
        float radius{};
        std::vector<Animation> Animations;
        std::string Path = "";

        std::vector<Meshlet> meshlets;
        uint32_t meshletVertexCount = 0;
        uint32_t meshletIndexCount = 0;
        uint32_t vertexOffset = 0;
        uint32_t indexOffset = 0;
        uint32_t meshletOffset = 0;

        Buffer vertexBuffer;
        Buffer meshletVertexBuffer;
        Buffer meshletIndexBuffer;
        bool loaded = false;
        void Load(Device* device, MeshletInfo* meshletInfos, uint32_t& meshletOffset);
        void Unload();

        static void CreateMeshFromFile(const char* filePath, std::shared_ptr<Mesh>& mesh);
        static void CreateMeshFromFile(const char *filePath, MeshData& meshData);
        static void CreateVerticesFromFile(const char* filePath, std::vector<Vertex>& vertices);
        static bool CreateQuad(std::vector<Vertex> &vertices, std::vector<Index> &indices, Index a, Index b, Index c, Index d);

        static void ExtractPitchYaw(glm::vec3& normal, uint16_t& pitch, uint16_t& yaw);
    private:
        void buildMeshlets();
        void buildMeshletsWithOptimizer();
    };
}
