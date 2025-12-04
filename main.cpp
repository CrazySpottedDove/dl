#include "dl/cli.h"
#include <filesystem>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "dl/tokenizer.h"
#include <fstream>
using namespace dl;

int main(int argc, char* argv[])
{
    // 输出到控制台
    const auto console = spdlog::stdout_color_mt("console");
    console->set_pattern("[%^%l %s:%#%$] %v");
    spdlog::set_default_logger(console);

    CliArgs args = cli::parse(argc, argv);

    if (args.show_help) {
        cli::print_help();
        return 0;
    }

    const auto& project_dir = args.project_dir;

    spdlog::debug("Project directory: {}", project_dir);

    // 遍历项目目录，读取目录下所有的lua文件，并对他们使用 tokenizer 进行词法分析


    // ...existing code...

    // 遍历项目目录，读取目录下所有的lua文件，并对他们使用 tokenizer 进行词法分析
    for (const auto& entry : std::filesystem::recursive_directory_iterator(project_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".lua") {
            spdlog::info("Processing file: {}", entry.path().string());
            std::ifstream file(entry.path());
            if (!file) {
                spdlog::error("Failed to open file: {}", entry.path().string());
                continue;
            }
            std::string   content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            dl::Tokenizer tokenizer(std::move(content));
            tokenizer.Print();   // 假设 Print() 会输出词法分析结果
        }
    }


    return 0;
}