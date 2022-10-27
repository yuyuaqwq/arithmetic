#include "parser.h"

namespace compiler {


Parser::Parser(Lexer* lexer) noexcept : m_lexer(lexer) { }


std::unique_ptr<S> Parser::ParseS() {
    return std::make_unique<S>(ParseAddExp());
}

std::unique_ptr<AddExp> Parser::ParseAddExp() {
    std::unique_ptr<MulExp> mulexp = ParseMulExp();
    std::vector<TokenType> oper1Arr;
    std::vector<std::unique_ptr<MulExp>> mulexpArr;

    do {
        Token token = m_lexer->LookAhead();
        if (token.type == TokenType::eof) {
            break;
        }
        if (token.type != TokenType::add && token.type != TokenType::sub) {
            break;
        }
        m_lexer->NextToken();
        oper1Arr.push_back(token.type);
        mulexpArr.push_back(ParseMulExp());

    } while (true);


    return std::make_unique<AddExp>(std::move(mulexp), std::move(oper1Arr), std::move(mulexpArr));
}

std::unique_ptr<MulExp> Parser::ParseMulExp() {
    std::unique_ptr<AtomicExp> atomicexp = ParseAtomicExp();
    std::vector<TokenType> oper2Arr;
    std::vector<std::unique_ptr<AtomicExp>> atomicexpArr;

    do {
        Token token = m_lexer->LookAhead();
        if (token.type == TokenType::eof) {
            break;
        }
        if (token.type != TokenType::mul && token.type != TokenType::div) {
            break;
        }
        m_lexer->NextToken();
        oper2Arr.push_back(token.type);
        atomicexpArr.push_back(ParseAtomicExp());

    } while (true);


    return std::make_unique<MulExp>(std::move(atomicexp), std::move(oper2Arr), std::move(atomicexpArr));
}

std::unique_ptr<AtomicExp> Parser::ParseAtomicExp() {
    Token token = m_lexer->LookAhead();
    if (token.type == TokenType::eof) {
        throw std::exception("parse atomic exp error");
    }
    std::unique_ptr<AddExp> addexp = nullptr;
    int num = 0;
    if (token.type == TokenType::lparen) {
        m_lexer->NextToken();
        addexp = ParseAddExp();
        m_lexer->MatchToken(TokenType::rparen);
    }
    else {
        num = ParseNumber();
    }
    return std::make_unique<AtomicExp>(std::move(addexp), num);
}


int Parser::ParseNumber() {
    Token token = m_lexer->NextToken();
    if (token.type != TokenType::num) {
        throw std::exception("parse number error");
    }
    int num = atoi(token.str.c_str());
    return num;
}


//ExpTail* Parser::ParseExpTail() {
//    Token token = m_lexer->NextToken();
//    if (token.type == TokenType::eof) {
//        return nullptr;
//    }
//    if (token.type != TokenType::add && token.type != TokenType::sub && token.type != TokenType::mul && token.type != TokenType::div) {
//        throw std::exception("parse exp tail error");
//    }
//    int num = ParseNumber();
//    ExpTail* expTail = ParseExpTail();
//    return new ExpTail(token.type, num, expTail);
//}


} // namespace compiler