#include "../Core/Headers/App.hpp"

using namespace AnA;

class MeshShaderApp : public App
{
public:
    MeshShaderApp() : App()
    {
        
    }
    ~MeshShaderApp()
    {

    }
protected:
    void onLoop()
    {

    }
};

std::vector<MeshInfo> meshInfos = 
{
    {"Models/cube.obj", {}}
};

float getHeight(int x, int z) {
    // Replace this with your terrain heightmap logic
    return sinf(x * 0.1f) * cosf(z * 0.8f) * tanf(random_double()) * 0.3f;
}

glm::vec3 calculateNormal(int x, int z) {
    // Get the height of the current point (x, z)
    float heightCenter = getHeight(x, z);

    // Get the height of the neighboring points
    float heightLeft = getHeight(x - 1, z);
    float heightRight = getHeight(x + 1, z);
    float heightUp = getHeight(x, z + 1);
    float heightDown = getHeight(x, z - 1);

    // Calculate the two vectors on the surface
    glm::vec3 v1(2.0f, heightRight - heightLeft, 0.0f);  // Horizontal vector (x-direction)
    glm::vec3 v2(0.0f, heightUp - heightDown, 2.0f);    // Vertical vector (z-direction)

    // Compute the cross product to get the normal vector
    glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

    return normal;
}

void GenTerrain(std::vector<Model::Vertex>& vertices, std::vector<uint32_t>& indices, float width, float height)
{
    for (float y = 0, x, z; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            z = getHeight(x, y);
            vertices.push_back({{x / width * 100.0f, z, y / height * 100.0f}, calculateNormal(x, y), 
                {(float)((uint32_t)x % 2), (float)((uint32_t)y % 2)}});
        }
    }
    for (uint32_t y = 0, x; y < static_cast<uint32_t>(height) - 1; y += 1)
    {
        for (x = 0; x < static_cast<uint32_t>(width) - 1; x += 1)
        {
            indices.push_back(x + y * static_cast<uint32_t>(width));
            indices.push_back(x + 1 + y * static_cast<uint32_t>(width));
            indices.push_back(x + (y + 1) * static_cast<uint32_t>(width));
            indices.push_back(x + (y + 1) * static_cast<uint32_t>(width));
            indices.push_back(x + 1 + y * static_cast<uint32_t>(width));
            indices.push_back(x + 1 + (y + 1) * static_cast<uint32_t>(width));
        }
    }
}

int main()
{
    std::vector<Model::Vertex> vertices;
    std::vector<uint32_t> indices;
    GenTerrain(vertices, indices, 1500.0f, 1500.0f);
    MeshShaderApp app;
    app.Init();
    auto& meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
    meshes.Append(vertices, indices, {{0.5, 0.5, 0.5}});
    //meshes.Append(meshInfos.data(), meshInfos.size());
    app.Run();
    return 0;
}