#pragma once

#include "Animation.hpp"

#include <functional>
#include <glm/fwd.hpp>
#include <glm/gtc/epsilon.hpp>
#include <vulkan/vulkan_core.h>

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>
#include <string>

namespace AnA
{
    struct Model
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

        struct ModelInfo
        {
            std::vector<Node> nodes;
            std::vector<Vertex> vertices;
            glm::vec3 minBounding;
            glm::vec3 maxBounding;
            Index indexStep;
            std::vector<Index> indices;
            std::string texturePath;
        };

        struct ModelStorageBufferObject
        {
            glm::mat4 model;
            static VkDescriptorSetLayoutBinding GetBindingDescriptionSet();
        };

        Model(const ModelInfo& modelInfo);
        ~Model();

        ModelInfo info{};
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


        static void CreateModelFromFile(const char* filePath, std::shared_ptr<Model>& model);
        static void CreateMeshFromFile(const char *filePath, ModelInfo& modelInfo);
        static void CreateVerticesFromFile(const char* filePath, std::vector<Vertex>& vertices);
        static bool CreateQuad(std::vector<Vertex> &vertices, std::vector<Index> &indices, Index a, Index b, Index c, Index d);
        static void CreateTerrainFromVertices(std::vector<Vertex>& vertices, std::vector<Index> &terrainVertices, size_t period);

        static void ExtractPitchYaw(glm::vec3& normal, uint16_t& pitch, uint16_t& yaw);
    private:
        void buildMeshlets();
        void buildMeshletsWithOptimizer();
    };
}
