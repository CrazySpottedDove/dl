#pragma once
#define WORK_MODE_COMPILE 0
#define WORK_MODE_FORMAT 1
#include <string>
namespace dl {
    struct CliArgs{
        std::string project_dir;
        std::string build_dir;
        bool show_help = false;
        bool show_version = false;
        int work_mode = WORK_MODE_COMPILE;
        std::string format_file;
    };

    namespace cli{
        CliArgs Parse(int argc, char* argv[]);
        void PrintHelp();
    }
}