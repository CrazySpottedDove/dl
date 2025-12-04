#include "dl/tokenizer.h"
#include "dl/token.h"
#include <cstdarg>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <magic_enum/magic_enum.hpp>
using namespace dl;

char Tokenizer::look(size_t offset) const noexcept
{
    offset += position_;
    return offset < length_ ? text_[offset] : DL_TOKENIZER_EOF;
}

char Tokenizer::get() noexcept
{
    return position_ < length_ ? text_[position_++] : DL_TOKENIZER_EOF;
}

bool Tokenizer::finished() const noexcept{
    return position_ >= length_;
}

void Tokenizer::add_token(const TokenType type, const size_t start_idx) noexcept
{
    tokens_.emplace_back(Token{type, text_.substr(start_idx, position_ - start_idx)});
}

int Tokenizer::get_long_string_delimiter_length() noexcept
{
    size_t init_pos = position_;
    while (look() == '=') {
        ++position_;
    }
    if (look() == '[') {
        ++position_;
        return position_ - init_pos - 1;
    }
    position_ = init_pos;
    return INVALID_LONG_STRING_DELIMITER_LENGTH;
}

void Tokenizer::get_long_string(const int delimeter_length)
{
    while (true) {
        const char c = get();
        if (c == DL_TOKENIZER_EOF) {
            error("Long string not closed");
        }
        if (c == ']') {
            bool ready_to_end = true;
            for (int i = 0; i < delimeter_length; ++i) {
                if (look(i) != '=') {
                    ready_to_end = false;
                    break;
                }
                ++position_;
            }
            if (ready_to_end && get() == ']') {
                return;
            }
        }
    }
}

Tokenizer::Tokenizer(const std::string&& text)
    : text_(text)
    , position_(0)
    , tokens_()
    , length_(text.length())
{
    tokenize();
}

void Tokenizer::tokenize()
{
    size_t token_start = 0;

    while (true) {
        // 标记当前解析循环中的空白字符起始位置
        token_start = position_;

        // 解析 WhiteSpace
        while (true) {
            if(finished()){
                break;
            }
            const char c = look();

            if (is_white_char(c)) {
                ++position_;
            }
            else {
                break;
            }
        }
        // 如果有空白字符，则生成对应的空白字符Token
        if (position_ > token_start) {
            add_token(TokenType::WhiteSpace, token_start);
            token_start = position_;
        }

        // 解析注释
        if (finished()) {
            return;
        }
        const char c = look();

        if (c == '-' && look(1) == '-') {
            position_ += 2;
            if (look() == '[') {
                ++position_;
                const int delimiter_length = get_long_string_delimiter_length();
                if (delimiter_length == INVALID_LONG_STRING_DELIMITER_LENGTH) {
                    // 普通注释
                    while (true) {
                        const char new_char = get();
                        if (new_char == DL_TOKENIZER_EOF || new_char == '\n') {
                            break;
                        }
                    }
                }
                else {
                    // 长注释
                    get_long_string(delimiter_length);
                }
            }else{
                // 普通注释
                while (true) {
                    const char new_char = get();
                    if (new_char == DL_TOKENIZER_EOF || new_char == '\n') {
                        break;
                    }
                }
            }
        }

        if (position_ > token_start) {
            add_token(TokenType::Comment, token_start);
            continue;
        }

        if(finished()){
            return;
        }
        const char c1 = get();

        if (c1 == '\'' || c1 == '\"') {
            // 字符串字面量
            while(true){
                if(finished()){
                    error("String not closed");
                }
                const char c2 = get();
                if(c2 == '\\'){
                    if(finished()){
                        error("String not closed");
                    }
                    const char c3 = get();
                    const auto escape_iter = character_for_escape.find(c3);
                    if(escape_iter == character_for_escape.end()){
                        error("Invalid escape sequence: \\%c", c3);
                    }
                }else if(c2 == c1){
                    break;
                }
            }
            add_token(TokenType::String, token_start);
            continue;
        }

        if(is_identifier_start_char(c1)){
            // 标识符或关键字
            while(is_identifier_char(look())){
                ++position_;
            }

            if(is_keyword(text_.substr(token_start, position_ - token_start))){
                add_token(TokenType::Keyword, token_start);
            }else{
                add_token(TokenType::Identifier, token_start);
            }
            continue;
        }

        // 数字
        if(is_digit_char(c1) || (c1 == '.' && is_digit_char(look()))){
            if (c1 == '0' && (look() == 'x')){
                ++position_;
                while(is_hex_digit_char(look())){
                    ++position_;
                }
            }else{
                while(is_digit_char(look())){
                    ++position_;
                }
                if(look() == '.'){
                    ++position_;
                    while(is_digit_char(look())){
                        ++position_;
                    }
                }
                if(look() == 'e' || look() == 'E'){
                    ++position_;
                    if (look() == '-'){
                        ++position_;
                    }
                    while(is_digit_char(look())){
                        ++position_;
                    }
                }
            }
            add_token(TokenType::Number, token_start);
            continue;
        }

        if (c1 == '['){
            const int delimiter_length = get_long_string_delimiter_length();
            if(delimiter_length == INVALID_LONG_STRING_DELIMITER_LENGTH){
                // 不是长字符串，作为单字符处理
                add_token(TokenType::Symbol, token_start);
            }else{
                // 长字符串
                get_long_string(delimiter_length);
                add_token(TokenType::String, token_start);
            }
            continue;
        }

        if(c1 == '.'){
            if(look() == '.'){
                get();
                if(look() == '.'){
                    get();
                }
            }
            add_token(TokenType::Symbol, token_start);
            continue;
        }

        if (is_equal_symbol_char(c1)){
            if(look() == '='){
                ++position_;
            }
            add_token(TokenType::Symbol, token_start);
            continue;
        }

        if (is_symbol_char(c1)){
            add_token(TokenType::Symbol, token_start);
            continue;
        }
        error("Bad Symbol %c in source code", c1);
    }
}

void Tokenizer::Print() const noexcept{
    for (const auto& token : tokens_) {
        printf("Type: %-12s, Text: %s\n", std::string(magic_enum::enum_name(token.type_)).c_str(), token.source_.c_str());
    }
}

void Tokenizer::error(const char* fmt, ...) const
{
    size_t line = 1;
    size_t col  = 1;
    for (size_t i = 0; i < position_; ++i) {
        if (text_[i] == '\n') {
            ++line;
            col = 1;
        }
        else {
            ++col;
        }
    }
    SPDLOG_ERROR("Tokenizer Error at Line {}, Column {}: ", line, col);

    char    buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    SPDLOG_ERROR("{}", buf);
    throw std::runtime_error("Tokenizer Error");
}
