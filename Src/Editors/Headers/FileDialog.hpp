#include <string>
#include <cstring>
#include <GLFW/glfw3.h>

namespace AnA
{
    namespace Editors
    {
        class FileDialog
        {
        public:
            FileDialog(std::string mode = "", std::string filter = "", std::string defaultPath = "") : Mode{mode}, Filter{filter}, Path{defaultPath}
            {

            }
            std::string Run()
            {
                char path[256] = "";
                std::string cmd = " --file-selection " + Mode;
                if (!Filter.empty())
                    cmd += " --file-filter=" + Filter;
#ifdef _WIN32
#else
                cmd = "/usr/bin/zenity" + cmd;
                if (!Filter.empty())
                    cmd += " --file-filter=" + Filter;
                FILE* f = popen(cmd.c_str(), "r");
                fgets(path, 256, f);
                size_t len = strlen(path);
                if (len <= 1)
                    return "";
                path[len - 1] = '\0';
                pclose(f);
#endif
                return std::string(path);
            }
            std::string Mode;
            std::string Filter;
            std::string Path;
        };
    }
}