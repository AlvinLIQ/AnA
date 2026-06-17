#include <string>
#include <cstring>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace AnA
{
    namespace Editor
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
#ifdef _WIN32
                cmd = "zenity" + cmd;
#else
                cmd = "/usr/bin/zenity" + cmd;
#endif
                if (!Filter.empty())
                    cmd += " --file-filter=\"" + Filter + "\"";
                FILE* f = popen(cmd.c_str(), "r");
                fgets(path, 256, f);
                size_t len = strlen(path);
                if (len <= 1)
                    return "";
                path[len - 1] = '\0';
                pclose(f);
                return std::string(path);
            }
            std::string Mode;
            std::string Filter;
            std::string Path;
        };
    }
}
