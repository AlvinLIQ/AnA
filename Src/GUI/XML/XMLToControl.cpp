#include "Headers/XMLToControl.hpp"
#include <rapidxml.hpp>
#include <iostream>
#include <fstream>

using namespace AnA;

void traverse_node(rapidxml::xml_node<> *node, int& id, int parent = -1) {
    if (!node)
        return;
    std::cout << node->name() << "* node" << id <<" = new "<< node->name() << "();\n";
    for (auto& attr : node->attributes())
    {
        if (attr.name() == "Name")
        {
            
        }
        else if (attr.name() == "Click")
        {
            std::cout << "node" << id << "->PointerEvents[PointerEventType::Released].emplace_back(" << attr.value() << ");\n";
        }
        else if (attr.name() == "Text")
        {
            std::cout << "node" << id << "->" << attr.name() << "(\"" << attr.value() << "\");\n";
        }
        else
        {
            std::cout << "node" << id << "->" << attr.name() << " = {" << attr.value() << "};\n";
        }
    }
    if (parent >= 0)
        std::cout << "node" << parent << "->Child(node" << id <<");\n";
    parent = id;
    for (auto& child : node->children())
    {
        id++;
        traverse_node(&child, id, parent);
    }
}

std::string XML::XMLToCode(std::string xml)
{
    std::string code = "";
    return code;
}

int main()
{
    std::ifstream file("Editor.xml");
    if (!file) {
        return 1;
    }

    // Read the entire file into a string (vector<char>)
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');

    rapidxml::xml_document<> doc;

    // Parse the XML from the string
    doc.parse<rapidxml::parse_fastest>(const_cast<char*>(buffer.data()));  // The const_cast is safe here as we need a mutable char*

    // Start traversal from the root node
    auto root = doc.first_node();
    int count = 0;
    traverse_node(root.get(), count);
    return 0;
}