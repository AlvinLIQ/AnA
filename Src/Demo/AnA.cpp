#include "../Core/Headers/AnA_App.hpp"
//#include "../GUI/Controls/Headers/AnA_Button.hpp"
#include "../VertexLoader/Headers/VertexLoader.hpp"

using namespace AnA;
//using namespace AnA::Controls;

void CopyVertices(IndexedVertex &indexedVertex, std::vector<AnA_Model::Vertex> &dstVertices)
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

int main()
{
    AnA_App* aApp = new AnA_App;
    aApp->Init();
    auto vData = AnA_Pipeline::ReadFile("Src/Demo/Cube_Vertices_Indexed.txt");
    IndexedVertex cube;
    LoadIndexedVerticesFromStr(vData.data(), &cube);
    //for (auto index : cube.indices)
    //    std::cout<<index<<std::endl;

    std::vector<AnA_Model::Vertex> vertices(cube.vertexCount);
    CopyVertices(cube, vertices);
    FreeVerticesMemory(reinterpret_cast<void**>(&cube));
    AnA_Object *object = new AnA_Object;
    object->Color = {0.1f, 0.2f, 0.3f};

    auto &objectProperties = object->Properties;
    objectProperties.sType = ANA_MODEL;
    objectProperties.transform.scale = {.7f, .7f, .7f};
    objectProperties.transform.rotation = glm::vec3(0.04f * glm::two_pi<float>(), 0.f, 0.f);
    objectProperties.transform.translation = {0.f, 0.f , 1.5f};
    
    AnA_App::CreateModel({vertices, cube.indexType, cube.indices}, object->Model);
    aApp->GetObjects().push_back(std::move(object));

/*
    AnA_Control::InitControl(aApp->GetSwapChain());
    AnA_Button *button = new AnA_Button;
    button->PrepareDraw();
    button->Model = AnA_App::Get2DModel();
    aApp->GetObjects().push_back(std::move(button));*/

    aApp->Run();
    delete aApp;
    aApp = nullptr;
    return 0;
}