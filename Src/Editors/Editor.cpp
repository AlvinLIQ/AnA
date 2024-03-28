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

int main()
{
    App* aApp = new App();
    aApp->Init();

    auto& SceneObjects = *Resource::ResourceManager::GetCurrent()->SceneObjects;

    Object* object = new Object;
    object->Color = {0.1f, 0.2f, 0.3f};

    auto &objectProperties = object->Properties;
    objectProperties.sType = ANA_MODEL;
    objectProperties.transform.scale = {.7f, .7f, .7f};
    objectProperties.transform.rotation = glm::vec3(0.04f * glm::two_pi<float>(), 0.f, 0.f);
    objectProperties.transform.translation = {0.f, 0.f , 1.5f};

    //object->Model = App::Get2DModel();
    Model::CreateModelFromFile(aApp->GetDevice(), "Models/torus.obj", object->Model);
    object->Texture = std::make_unique<Texture>("Textures/texture.png", aApp->GetDevice());

    SceneObjects.Append(std::move(object));

    Object* nObj = new Object;
    nObj->Color = {1.0, 1.0, 0.0};
    nObj->Properties = objectProperties;
    //nObj->Properties.transform.scale = {.0001, .0001, .0001};
    nObj->Properties.transform.rotation = {glm::two_pi<float>() / 4, 0.f, 0.f};
    
    nObj->Properties.transform.translation = {-1.5, -.5, 1.5};
    App::CreateCubeModel(nObj->Model);
    //nObj->Texture = std::make_unique<Texture>("Textures/test.jpg", aApp->GetDevice());
    SceneObjects.Append(nObj);

    nObj = new Object;
    App::CreateCubeModel(nObj->Model);
    nObj->Properties.sType = ANA_MODEL;
    nObj->Color = {0.5f, 0.5f, .5f};
    nObj->Properties.transform.scale = {11.4f, 0.02f, 11.4};
    nObj->Properties.transform.translation = {3.0, 0.5, 0.0};
    SceneObjects.Append(nObj);
/*
    nObj = new Object;
    Model::CreateModelFromFile(aApp->GetDevice(), "Models/sphere.obj", nObj->Model);
    nObj->Properties.sType = ANA_MODEL;
    nObj->Color = {1.0F, 1.0f, 1.0f};
    nObj->Properties.transform.scale = {0.4f, 0.4f, 0.4};
    nObj->Properties.transform.translation = {0.0, -0.2, 0.0};
    SceneObjects.Append(nObj);*/

    nObj = new Object;
    App::CreateCubeModel(nObj->Model);
    nObj->Properties.sType = ANA_MODEL;
    nObj->Color = {1.0F, 1.0f, 1.0f};
    nObj->Properties.transform.scale = {0.4f, 0.4f, 0.4};
    nObj->Properties.transform.translation = {1.5, 0.3, 0.0};
    SceneObjects.Append(nObj);

    nObj = new Object;
    nObj->Model = aApp->Get2DModel();
    nObj->Properties.sType = ANA_RECTANGLE;
    nObj->Color = {1.0F, 1.0f, 1.0f};
    nObj->Properties.transform.scale = {0.4f, 0.3f, 0.4};
    nObj->Properties.transform.translation = {-0.6, -.7, 0.0};
    SceneObjects.Append(nObj);
/*
    nObj = new Object;
    nObj->Model = aApp->Get2DModel();
    nObj->Properties.sType = ANA_RECTANGLE;
    nObj->Color = {1.0F, 1.0f, 1.0f};
    nObj->Properties.transform.scale = {0.02f, 1.0f, 0.4};
    nObj->Properties.transform.translation = {0.0, 0.0, 0.0};
    SceneObjects.Append(nObj);

    nObj = new Object;
    nObj->Model = aApp->Get2DModel();
    nObj->Properties.sType = ANA_RECTANGLE;
    nObj->Color = {1.0F, 1.0f, 1.0f};
    nObj->Properties.transform.scale = {1.0f, 0.02f, 0.4};
    nObj->Properties.transform.translation = {0.0, 0.0, 0.0};
    SceneObjects.Append(nObj);*/

/*  
    Control::InitControl(&aApp->GetSwapChain());
    
    Button* button = new Button;
    button->Model = App::Get2DModel();
    button->HorizontalAlignment = AlignmentType::Start;
    button->VerticalAlignment = AlignmentType::Start;
    button->Color = glm::vec3(1.0f);
    button->Properties.color = glm::vec3(1.0f);
    //button->Texture = nObj->Texture;
    SceneObjects.Append(button);

    TextBlock* textBlock = new TextBlock;
    textBlock->Model = App::Get2DModel();
    textBlock->Text("Test123");
    textBlock->Properties.transform.scale = glm::vec3(.3f);
    textBlock->Properties.color = glm::vec3(1.0);

    SceneObjects.Append(textBlock);
    */
    aApp->Run();
    delete aApp;
    aApp = nullptr;
    return 0;
}