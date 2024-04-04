#include "Headers/Editor.hpp"

using namespace AnA;
using namespace Editors;

Editor::Editor()
{

}

Editor::~Editor()
{

}

void Editor::Init()
{

}

#define INCLUDE_STB_IMAGE

#include "../Core/Headers/App.hpp"
//#include "../GUI/Controls/Headers/Button.hpp"
//#include "../GUI/Controls/Headers/TextBlock.hpp"
#include "../VertexLoader/Headers/VertexLoader.hpp"

using namespace AnA;
//using namespace AnA::Controls;

void CopyVertices(IndexedVertex &indexedVertex, std::vector<Model::Vertex> &dstVertices)
{
    /*
    auto &srcVertices = indexedVertex.vertices;
    for (int i = 0; i < indexedVertex.vertexCount; i++)
    {
        dstVertices[i].position = glm::vec3(srcVertices[i].position[0], srcVertices[i].position[1], srcVertices[i].position[2]);
        //printf("%f %f %f\n", srcVertices[i].position[0], srcVertices[i].position[1], srcVertices[i].position[2]);
        dstVertices[i].color = glm::vec3(srcVertices[i].color[0], srcVertices[i].color[1], srcVertices[i].color[2]);
    }*/
    memcpy(dstVertices.data(), indexedVertex.vertices, sizeof(Vertex_T) * indexedVertex.vertexCount);
}

const VkDeviceSize offset = 0;
std::vector<MeshInfo> meshInfos = 
{
    {"Models/cube.obj", {{3.0, 0.5, 0.0}, {11.4f, 0.02f, 11.4}}},
    {"Models/torus.obj", {{0.f, 0.f , 1.5f}, {.7f, .7f, .7f}}},
    {"Models/cube.obj", {{-1.5, -.5, 1.5}, {0.7f, 0.7f, 0.7f}}},
    {"Models/cube.obj", {{1.5, -0.4, 0.0}, {0.4f, 0.4f, 0.4}}}
};

int main()
{
    App app{};
    app.Init();
    auto meshes = Resource::ResourceManager::GetCurrent()->SceneObjects;
    //std::vector<std::string> files(1000, "Models/cube.obj");
    for (int i = 0; i < 1000; i++)
        meshInfos.push_back({"Models/cube.obj", {{drand48(), drand48(), drand48()}, {drand48(), drand48(), drand48()}}});
    meshes->Append(meshInfos);
    app.Run();
    return 0;
}