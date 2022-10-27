#ifndef COMPILER_PARSER_H_
#define COMPILER_PARSER_H_

#include <string>
#include <vector>
#include <memory>

#include "compiler/Lexer.h"
#include "compiler/ast.h"

namespace compiler {


class Parser {
public:
    Parser(Lexer* lexer) noexcept;

public:

    std::unique_ptr<S> ParseS();

    std::unique_ptr<AddExp> ParseAddExp();

    std::unique_ptr<MulExp> ParseMulExp();

    std::unique_ptr<AtomicExp> ParseAtomicExp();


    int ParseNumber();

private:
    Lexer* m_lexer;

};


} // namespace compiler

#endif // COMPILER_PARSER_H_