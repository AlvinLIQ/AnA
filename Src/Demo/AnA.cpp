#include "../Core/Headers/AnA_App.h"
#include "../VertexLoader/Headers/VertexLoader.h"

using namespace AnA;

int main()
{
    AnA_App* aApp = new AnA_App;
    aApp->Init();

    auto vData = AnA_Pipeline::ReadFile("Src/Demo/Cube_Vertices.txt");
    Vertex cube;
    int vCount;
    LoadVerticesFromStr(vData.data(), &cube, &vCount);

    std::vector<AnA_Model::Vertex> vertices(vCount);
    for (int i = 0; i < vCount; i++)
    {
        vertices[i].position = glm::vec3(cube[i].position[0], cube[i].position[1], cube[i].position[2]);
        //printf("%f %f %f\n", cube[i].position[0], cube[i].position[1], cube[i].position[2]);
        vertices[i].color = glm::vec3(cube[i].color[0], cube[i].color[1], cube[i].color[2]);
    }
    FreeVerticesMemory(reinterpret_cast<void**>(&cube));
    AnA_Object *object = new AnA_Object;
    object->Color = {0.1f, 0.2f, 0.3f};

    ItemProperties objectProperties;
    objectProperties.sType = ANA_MODEL;
    objectProperties.transform.scale = {.4f, .4f, .4f};
    objectProperties.transform.rotation = glm::vec3(0.04f * glm::two_pi<float>(), 0.f, 0.f);
    objectProperties.transform.translation = {0.f, 0.f , 1.5f};
    object->ItemsProperties.push_back(std::move(objectProperties));
    
    AnA_App::CreateModel(vertices, &object->Model);
    aApp->GetObjects().push_back(std::move(object));

    aApp->Run();
    delete aApp;
    aApp = nullptr;
    return 0;
}