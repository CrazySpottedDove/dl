#pragma once
#include "dl/token.h"
#include <vector>

namespace dl {
#define DL_TOKENIZER_EOF '\0'
#define INVALID_LONG_STRING_DELIMITER_LENGTH -1
class Tokenizer
{
public:
    Tokenizer(const std::string&& text, const int work_mode);

    void Print() const noexcept;
    std::vector<Token>& getTokens() noexcept { return tokens_; }
    std::vector<Token>& getCommentTokens() noexcept { return comment_tokens_; }
private:
    // 查看当前位置往前看第offset个字符
    char peek(size_t offset = 0) const noexcept;

    // 获取当前字符并往前走一位
    char get() noexcept;

    void tokenize_compile();
    void tokenize_format();

    void addToken(const TokenType type, const size_t start_idx) noexcept;

    bool finished() const noexcept;
    /**
     * @brief Get the long string delimiter length object
     *
     * @return int
     * @note 在已经消费了一个 '[' 时调用
     * @note 如果是长字符串的开始，消费完长字符串的起始定界符并返回定界符长度
     * @note 如果不是长字符串的开始，返回 INVALID_LONG_STRING_DELIMITER_LENGTH，且不消费任何字符
     */
    int getLongStringDelimiterLength() noexcept;

    /**
     * @brief Get the long string object
     *
     * @param delimiter_length
     * @note 若读到 EOF，抛出错误
     */
    void getLongString(const int delimiter_length);

    // 接收类似于 printf 接收的参数
    void error( const char* fmt, ... ) const;

    std::string        text_;
    size_t             position_ = 0;
    std::vector<Token> tokens_;
    std::vector<Token> comment_tokens_;
    size_t             length_ = 0;
    size_t line_ = 1;
};
}   // namespace dl