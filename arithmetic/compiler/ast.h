#ifndef COMPILER_AST_H_
#define COMPILER_AST_H_

#include <string>
#include <vector>
#include <memory>

#include "compiler/lexer.h"

namespace compiler {


struct AddExp;

struct AtomicExp {
    std::unique_ptr<AddExp> addexp;
    int num;

    AtomicExp(std::unique_ptr<AddExp> t_addexp, int t_num) noexcept;
};

struct MulExp {
    std::unique_ptr<AtomicExp> atomicexp;
    std::vector<TokenType> oper2Arr;
    std::vector<std::unique_ptr<AtomicExp>> atomicexpArr;

    MulExp(std::unique_ptr<AtomicExp> t_atomicexp, std::vector<TokenType>&& t_oper2Arr, std::vector<std::unique_ptr<AtomicExp>>&& t_atomicexpArr);
};



struct AddExp {
    std::unique_ptr<MulExp> mulexp;
    std::vector<TokenType> oper1Arr;
    std::vector<std::unique_ptr<MulExp>> mulexpArr;
    AddExp(std::unique_ptr<MulExp> t_mulexp, std::vector<TokenType>&& t_oper1Arr, std::vector<std::unique_ptr<MulExp>>&& t_mulexpArr);
};

struct S {
    std::unique_ptr<AddExp> addexp;
    S(std::unique_ptr<AddExp> t_addexp) noexcept;
};


} // namespace compiler

#endif // COMPILER_AST_H_