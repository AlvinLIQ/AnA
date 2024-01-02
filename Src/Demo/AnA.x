#include "../Core/Headers/App.hpp"
#include "test.h"
#include "../GUI/Controls/Headers/Button.hpp"
#include "../GUI/Controls/Headers/TextBlock.hpp"
#include "../VertexLoader/Headers/VertexLoader.hpp"

using namespace AnA;
using namespace AnA::Controls;

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

int main()
{
    App* aApp = new App;
    aApp->Init();

    Object* object = new Object;
    object->Color = {0.1, 0.6, 0.0};

    auto &objectProperties = object->Properties;
    objectProperties.sType = ANA_MODEL;
    objectProperties.transform.scale = {.01f, .01f, .0001f};
    objectProperties.transform.rotation = {};
    objectProperties.transform.translation = {0.f, 0.f , 1.5f};

    //object->Model = App::Get2DModel();
    //Model::CreateModelFromFile(aApp->GetDevice(), "Models/torus.obj", object->Model);
    Model::ModelInfo modelInfo{};
    glm::vec3 yellow{0.8f, 0.8f, 0.0f};
    double x,y;
    for (int i = 0; i < numsof(coords); i+=3)
    {
        Model::Vertex vertex{};
        x = 6731.0 * coords[i] - 817062.0;
        y = -6731.0 * log(glm::abs(tan(coords[i + 1]) / 2.0f + glm::pi<double>() / 4.0)) - 1355.5;
        //printf("%f %f\n", x, y);
        vertex.color = i & 1 ? object->Color : yellow;
        vertex.position = {x, y, -0.046091f};
        modelInfo.vertices.push_back(vertex);
    }
    printf("vertex count:%zu\n", modelInfo.vertices.size());
    object->Model = std::make_shared<Model>(aApp->GetDevice(), modelInfo);
    object->Texture = std::make_unique<Texture>("Textures/texture.png", aApp->GetDevice());

    aApp->SceneObjects.Append(std::move(object));
/*  
    Control::InitControl(&aApp->GetSwapChain());
    
    Button* button = new Button;
    button->Model = App::Get2DModel();
    button->HorizontalAlignment = AlignmentType::Start;
    button->VerticalAlignment = AlignmentType::Start;
    button->Color = glm::vec3(1.0f);
    button->Properties.color = glm::vec3(1.0f);
    //button->Texture = nObj->Texture;
    aApp->SceneObjects.Append(button);

    TextBlock* textBlock = new TextBlock;
    textBlock->Model = App::Get2DModel();
    textBlock->Text("Test123");
    textBlock->Properties.transform.scale = glm::vec3(.3f);
    textBlock->Properties.color = glm::vec3(1.0);

    aApp->SceneObjects.Append(textBlock);
    */
    aApp->Run();
    delete aApp;
    aApp = nullptr;
    return 0;
}