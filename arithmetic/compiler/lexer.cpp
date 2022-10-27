#include "lexer.h"

namespace compiler {


Lexer::Lexer(std::string src) : m_src(src), m_idx(0) {
    m_nextToken.type = TokenType::none;
}


char Lexer::NextChar() noexcept {
    if (m_idx < m_src.size()) {
        return m_src[m_idx++];
    }
    return 0;
}

void Lexer::SkipChar(int c) noexcept {
    m_idx += c;
}

Token Lexer::LookAhead() {
    if (m_nextToken.type == TokenType::none) {
        m_nextToken = NextToken();
    }
    return m_nextToken;

}

Token Lexer::NextToken() {

    Token token;
    if (m_nextToken.type != TokenType::none) {
        token = m_nextToken;
        m_nextToken.type = TokenType::none;
        return token;
    }


    char c;

    while ((c = NextChar()) && c == ' ');

    if (c == 0) {
        token.type = TokenType::eof;
        return token;
    }

    switch (c) {
    case '+':
        token.type = TokenType::add;
        return token;
    case '-':
        token.type = TokenType::sub;
        return token;
    case '*':
        token.type = TokenType::mul;
        return token;
    case '/':
        token.type = TokenType::div;
        return token;
    case '(':
        token.type = TokenType::lparen;
        return token;
    case ')':
        token.type = TokenType::rparen;
        return token;
    }


    if (c < '0' || c > '9') {
        throw std::exception("cannot parse token");
    }
    token.type = TokenType::num;
    do {
        if (c >= '0' && c <= '9') {
            token.str.push_back(c);
        }
        else {
            SkipChar(-1);
            break;
        }
    } while (c = NextChar());

    return token;
}

void Lexer::MatchToken(TokenType type) {
    Token token = NextToken();
    if (token.type != type) {
        throw std::exception("cannot match token");
    }
    return;
}

} // namespace compiler