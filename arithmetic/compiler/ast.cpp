#include "ast.h"


namespace compiler {
	AtomicExp::AtomicExp(std::unique_ptr<AddExp> t_addexp, int t_num) noexcept : addexp(std::move(t_addexp)), num(t_num) { }

	MulExp::MulExp(std::unique_ptr<AtomicExp> t_atomicexp, std::vector<TokenType>&& t_oper2Arr, std::vector<std::unique_ptr<AtomicExp>>&& t_atomicexpArr) : atomicexp(std::move(t_atomicexp)), oper2Arr(std::move(t_oper2Arr)), atomicexpArr(std::move(t_atomicexpArr)) { }

	AddExp::AddExp(std::unique_ptr<MulExp> t_mulexp, std::vector<TokenType>&& t_oper1Arr, std::vector<std::unique_ptr<MulExp>>&& t_mulexpArr) : mulexp(std::move(t_mulexp)), oper1Arr(std::move(t_oper1Arr)), mulexpArr(std::move(t_mulexpArr)) { }

	S::S(std::unique_ptr<AddExp> t_addexp) noexcept : addexp(std::move(t_addexp)) { }

} // namespace compiler