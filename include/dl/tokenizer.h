#pragma once
#include "dl/token.h"
#include <vector>

namespace dl {
#define DL_TOKENIZER_EOF '\0'
#define INVALID_LONG_STRING_DELIMITER_LENGTH -1
class Tokenizer
{
public:
    Tokenizer(const std::string&& text);

    void Print() const noexcept;
private:
    // 查看当前位置往前看第offset个字符
    char look(const size_t look_offset = 0) const noexcept;

    // 获取当前字符并往前走一位
    char get() noexcept;

    void tokenize();

    void add_token(const TokenType type, const size_t start_idx) noexcept;

    bool finished() const noexcept;
    /**
     * @brief Get the long string delimiter length object
     *
     * @return int
     * @note 在已经消费了一个 '[' 时调用
     */
    int get_long_string_delimiter_length() noexcept;

    /**
     * @brief Get the long string object
     *
     * @param delimiter_length
     * @note 若读到 EOF，抛出错误
     */
    void get_long_string(const int delimiter_length);

    // 接收类似于 printf 接收的参数
    void error( const char* fmt, ... ) const;

    std::string        text_;
    size_t             position_ = 0;
    std::vector<Token> tokens_;
    size_t             length_ = 0;
};
}   // namespace dl