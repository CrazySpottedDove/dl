#pragma once

#include <string>
namespace dl {
    struct CliArgs{
        std::string project_dir;
        std::string build_dir;
        bool show_help = false;
        bool show_version = false;
    };

    namespace cli{
        CliArgs parse(int argc, char* argv[]);
        void print_help();
    }
}