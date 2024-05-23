#pragma once
#include <cstdint>
#include "../../Core/Resources/Headers/Mesh.hpp"


namespace AnA
{
    namespace Engine
    {
        struct ProjectHeader
        {
            uint32_t type;
            char name[32];
            uint32_t startupSceneId;
        };

        struct TextInfo
        {
            int width;
            int height;
            float lineHeight;
            String text;
        };

        struct Scene
        {
            //Textures
            Vector<String> texturePaths;
            //Shaders
            Vector<String> codes;
            //MeshInfos
            Vector<MeshInfo> meshInfos;
        };

        struct ProjectInfo
        {
            ProjectHeader header;
            Vector<Scene> scenes;
        };

        class Project
        {
        public:
            Project(ProjectInfo projInfo);
        };
    }
}