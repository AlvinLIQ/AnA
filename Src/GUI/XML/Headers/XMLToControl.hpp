#pragma once
#include <string>
#include <filesystem>

namespace AnA
{
    namespace Controls
    {
        class Control;
    }
    namespace XML
    {
        inline std::string XMLToCode(std::filesystem::path& path);
    }
}