#pragma once
#include "dl/ast.h"
#include "dl/token.h"
#include <ostream>

namespace dl {
namespace AstTools {
class AstPrinter
{
public:
    AstPrinter(std::ostream& out)
        : out_(out)
        , indent_(0)
    {}

    void PrintAst(const AstNode* ast);
    void PrintAst(const AstNode* ast, const std::vector<CommentToken>& comment_tokens);

private:
    void         print_token(const Token* token);
    void         print_token_format_mode(const Token* token);
    void         print_expr(const AstNode* expr);
    void         print_expr_format_mode(const AstNode* expr);
    void         print_stat(const AstNode* stat);
    void         print_stat_format_mode(const AstNode* stat);
    const CommentToken* comment_token();
    inline void  indent()
    {
        for (int i = 0; i < indent_; ++i) {
            out_.put('\t');
        }
    }
    inline void space() { out_ .put (' '); }
    inline void breakline() { out_.put('\n'); }

    void        breakline_format_mode();
    bool        is_block_stat(AstNodeType type);
    inline void inc_indent() { ++indent_; is_block_start_ = true; }
    inline void dec_indent() { --indent_; is_block_start_ = false;}

    std::ostream&             out_;
    int                       indent_;
    std::size_t               line_               = 1;
    std::size_t               comment_index_      = 0;
    const std::vector<CommentToken>* comment_tokens_     = nullptr;
    bool                      line_start_         = true;
    bool                      last_is_block_stat_ = false;
    bool                      is_block_start_     = true;
};

void PrintAst(const AstNode* ast, std::ostream& out);
void PrintAst(const AstNode* ast, const std::vector<CommentToken>& comment_tokens, std::ostream& out);
}   // namespace AstTools
}   // namespace dl