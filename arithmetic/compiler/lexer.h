#ifndef COMPILER_LEXER_H_
#define COMPILER_LEXER_H_

#include <string>

namespace compiler {

enum class TokenType {
    none = 0,
    eof,
    add,
    sub,
    mul,
    div,
    lparen,
    rparen,
    num,
};

struct Token {
    TokenType type;
    std::string str;
};

class Lexer {
public:
    Lexer(std::string src);

public:
    char NextChar() noexcept;

    void SkipChar(int c) noexcept;

    Token LookAhead();

    Token NextToken();

    void MatchToken(TokenType type);


private:
    std::string m_src;
    size_t m_idx;
    Token m_nextToken;
};

} // namespace compiler

#endif // COMPILER_LEXER_H_