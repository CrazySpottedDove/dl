#include "dl/cli.h"
#include <cstdio>

using namespace dl;

CliArgs cli::parse(int argc, char* argv[]){
    CliArgs args;

    for(int i = 1; i < argc; ++i){
        std::string arg = argv[i];
        if(arg == "--help" || arg == "-h"){
            args.show_help = true;
            break;
        } else if(arg == "--version" || arg == "-v"){
            args.show_version = true;
            break;
        } else if(arg == "--project-dir"){
            if(i + 1 < argc){
                args.project_dir = argv[++i];
            }
        } else if(arg == "--build-dir"){
            if(i + 1 < argc){
                args.build_dir = argv[++i];
            }
        }
    }
    return args;
}

void cli::print_help(){
    printf("Usage: dl [options]\n"
           "Options:\n"
           "  -h, --help            Show this help message\n"
           "  -v, --version         Show version information\n"
           "  --project-dir <path>  Specify the project directory\n"
           "  --build-dir <path>    Specify the build directory\n");
}
