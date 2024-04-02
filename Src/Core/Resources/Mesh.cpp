#include "Headers/Mesh.hpp"

using namespace AnA;

Mesh::Mesh(std::string filePath)
{
    Model::GetVerticesFromFile(filePath.c_str(), Vertices);
}

Mesh::~Mesh()
{

}