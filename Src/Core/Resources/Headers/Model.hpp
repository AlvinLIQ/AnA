#pragma once

#include <functional>
#include <glm/fwd.hpp>
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
            glm::vec3 normal{};
            glm::vec2 uv{};

            bool operator==(const Vertex& vertex) const
            {
                return position == vertex.position && normal == vertex.normal && uv == vertex.uv;
            }
            static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
        };
        struct VertexHash
        {
            std::size_t operator() (const Vertex& vertex) const
            {
                return std::hash<float>()(vertex.position.x) ^ std::hash<float>()(vertex.position.y) ^ std::hash<float>()(vertex.position.z)
                    ^ std::hash<float>()(vertex.normal.x)
                    ^ std::hash<float>()(vertex.normal.y)
                    ^ std::hash<float>()(vertex.normal.z)
                    ^ std::hash<float>()(vertex.uv.x)
                    ^ std::hash<float>()(vertex.uv.y);
            }
        };

        struct Meshlet
        {
            uint32_t vertices[128];
            uint8_t indices[256 * 3];
            uint32_t indexCount;
            uint32_t vertexCount;
            uint32_t farVertexID;
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
        
        struct ModelInfo
        {
            std::vector<Vertex> vertices;
            std::vector<glm::vec2> vertexProjections;
            Index indexStep;
            std::vector<Index> indices;
        };

        struct ModelStorageBufferObject
        {
            glm::mat4 model;
            static VkDescriptorSetLayoutBinding GetBindingDescriptionSet();
        };

        Model(const ModelInfo& modelInfo);
        ~Model();

        ModelInfo info{};
        std::string Path = "";

        std::vector<Meshlet> meshlets;
        uint32_t meshletVertexCount = 0;
        uint32_t meshletIndexCount = 0;

        static void CreateModelFromFile(const char* filePath, std::shared_ptr<Model>& model);
        static void CreateMeshFromFile(const char *filePath, std::vector<Vertex>& vertices, std::vector<Index>& indices, size_t vertexOffset = 0);
        static void CreateVerticesFromFile(const char* filePath, std::vector<Vertex>& vertices);
        static bool CreateQuad(std::vector<Vertex> &vertices, std::vector<Index> &indices, Index a, Index b, Index c, Index d);
        static void CreateTerrainFromVertices(std::vector<Vertex>& vertices, std::vector<Index> &terrainVertices, size_t period);
    private:
        void buildMeshlets();
        void buildMeshletsWithOptimizer();
    };
}
