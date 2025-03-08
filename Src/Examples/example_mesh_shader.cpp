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

void GenTerrain(std::vector<glm::vec3>& vertices, float width, float height)
{
    for (float y = 0, x, z; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            z = sin(x) * cos(y) * random_double();
            vertices.push_back({x, y, z});
        }
    }
}

int main()
{
    MeshShaderApp app;
    app.Init();
    auto& meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
    meshes.Append(meshInfos.data(), meshInfos.size());
    app.Run();
    return 0;
}