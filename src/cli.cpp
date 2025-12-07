#include "dl/cli.h"
#include <cstdio>

using namespace dl;

CliArgs cli::Parse(int argc, char* argv[]){
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
        }else if(arg == "--format"){
            if(i + 1 < argc){
                args.format_file = argv[++i];
                args.work_mode = WORK_MODE_FORMAT;
            }
        }
    }
    return args;
}

void cli::PrintHelp(){
    std::printf("Usage: dl [options]\n");
    std::printf("Options:\n");
    std::printf("  -h, --help            Show this help message and exit\n");
    std::printf("  -v, --version         Show version information and exit\n");
    std::printf("  --project-dir <path>  Specify the project directory\n");
    std::printf("  --build-dir <path>    Specify the build directory\n");
    std::printf("  --format <file>       Format the specified file\n");
}
