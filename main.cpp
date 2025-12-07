#include "dl/cli.h"
#include <filesystem>
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "dl/parser.h"
#include "dl/tokenizer.h"
#include "dl/ast_tools.h"
#include <fstream>
using namespace dl;

int main(int argc, char* argv[])
{
    // 输出到控制台
    const auto console = spdlog::stdout_color_mt("console");
    console->set_pattern("[%^%l %s:%#%$] %v");
    spdlog::set_default_logger(console);

    CliArgs args = cli::Parse(argc, argv);

    if (args.show_help) {
        cli::PrintHelp();
        return 0;
    }

    if(args.work_mode == WORK_MODE_FORMAT){
        const auto& file_path = args.format_file;
        if(file_path.empty()){
            SPDLOG_ERROR("No file specified for formatting.");
            return 1;
        }
        std::ifstream file(file_path);
        if (!file) {
            SPDLOG_ERROR("Failed to open file: {}", file_path.c_str());
            return 1;
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        Tokenizer tokenizer(std::move(content), WORK_MODE_FORMAT);
        Parser parser(tokenizer.getTokens());
        std::ofstream out_file(file_path);
        AstTools::PrintAst(parser.GetAstRoot(), tokenizer.getCommentTokens(), out_file);
    }
    else if (args.work_mode == WORK_MODE_COMPILE){
        const auto& project_dir = args.project_dir;
        const auto& build_dir   = args.build_dir;

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
                Tokenizer tokenizer(std::move(content), WORK_MODE_COMPILE);
                // tokenizer.Print();   // 假设 Print() 会输出词法分析结果

                Parser parser(tokenizer.getTokens());

                std::ofstream out_file(build_dir / entry.path().filename());

                AstTools::PrintAst(parser.GetAstRoot(), out_file);
            }
        }
    }




    return 0;
}