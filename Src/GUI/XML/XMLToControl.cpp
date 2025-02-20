#include "Headers/XMLToControl.hpp"
#include <rapidxml.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <set>

using namespace AnA;

void traverse_node(rapidxml::xml_node<> *node, std::set<std::string>& usedNodes, std::stringstream& ss, int& id, int parent = -1) {
    if (!node)
        return;
    if (usedNodes.find(std::string(node->name())) == usedNodes.end())
    {
        usedNodes.insert(std::string(node->name()));
    }
    ss << '\t' << node->name() << "* node" << id <<" = new "<< node->name() << "();\n";
    for (auto& attr : node->attributes())
    {
        ss << '\t';
        if (attr.name() == "Name")
        {
            ss << "controlMap.insert(std::pair<std::string, Controls::Control*>(\"" 
                << attr.value() << "\", (Controls::Control*)node" << id << "));\n"; 
        }
        else if (attr.name() == "Click")
        {
            ss << "node" << id << "->PointerEvents[PointerEventType::Released].emplace_back(" << attr.value() << ");\n";
        }
        else if (attr.name() == "Text")
        {
            ss << "node" << id << "->" << attr.name() << "(\"" << attr.value() << "\");\n";
        }
        else if (attr.name() == "ImageInfo" || attr.name() == "RenderMode")
        {
            ss << "node" << id << "->" << attr.name() << "(" << attr.value() << ");\n";
        }
        else
        {
            ss << "node" << id << "->" << attr.name() << " = {" << attr.value() << "};\n";
        }
    }
    if (parent >= 0)
        ss << '\t' << "node" << parent << "->Child(node" << id <<");\n";
    parent = id;
    for (auto& child : node->children())
    {
        id++;
        traverse_node(&child, usedNodes, ss, id, parent);
    }
}

std::string XML::XMLToCode(std::string path)
{
    std::ifstream file(path);
    if (!file) {
        return "";
    }

    // Read the entire file into a string (vector<char>)
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');

    rapidxml::xml_document<> doc;

    // Parse the XML from the string
    doc.parse<rapidxml::parse_fastest>(buffer.data());

    // Start traversal from the root node
    auto root = doc.first_node();
    int count = 0;
    std::string code = "";
    std::stringstream ss{};
    std::set<std::string> usedNodes{};
    traverse_node(root.get(), usedNodes, ss, count);
    ss.flush();
    for (auto& node : usedNodes)
    {
        code += "#include \"../GUI/Controls/Headers/" + node + ".hpp\"\n";
    }
    auto _path = std::filesystem::path(path);
    std::string className = _path.filename().replace_extension().string();
    code += "#include \"Headers/" + className + ".hpp\"\n\nusing namespace AnA;\nusing namespace Controls;\nusing namespace " + _path.parent_path().filename().string() + ";\n";

    code += "\nControls::Control* " + className + "::InitControl()\n{\n" + ss.str() + "\n\treturn node0;\n}\n";

    std::fstream fs(_path.replace_extension(".g.cpp"), std::ios_base::out);
    fs.write(code.c_str(), code.length());

    return code;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 2;
    std::cout << XML::XMLToCode(argv[1]);
    return 0;
}