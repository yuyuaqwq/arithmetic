// arithmetic.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include <string>
#include <vector>

/*

-----------------------------------

S -> exp
exp -> exp operator exp | number
operator -> + | - | * | /


1 + 2
    -> S
        -> exp
            -> exp + exp
                -> 1 + 2

1 + 2 - 1
    -> S
        -> exp
            -> exp operator exp
                -> 此时存在歧义，无法确定应当将第一个exp展开为1 + 2，还是将第二个exp展开为2 - 1（我暂时觉得不会是有歧义的，除非推导过程是没有优先级的，但实际代码不可能没有优先级，但是我学习的那篇文章这样子讲）



!!!!尝试消除左递归，可以先跳过!!!!
由于 exp 产生的句型一定是 number { operator number }

修改推导式，使exp产生以number开头的句子
    exp -> number exp'
添加推导式exp'，用于推导不定数量的operator number，其中ε是空的意思
    exp' -> operator exp' | ε

S -> exp
exp -> number exp'
exp' -> operator number exp' | ε
operator -> + | - | * | /


1 + 2
    -> S
        -> exp
            -> 1 exp'
                -> 1 + 2 exp'
                    -> 1 + 2

1 + 2 - 1
    -> S
        -> exp
            -> 1 exp'
                -> 1 + 2 exp'
                    -> 1 + 2 - 1 exp'
                        -> 1 + 2 - 1


抽象语法树：
          +
        /   \
       1     -
           /   \
          2     1
左结合变为了右结合


用EBNF可以描述成
S -> exp
exp -> number { operator number }
operator -> + | - | * | /

1 + 2 - 1
    -> S
        -> exp
            -> number + number - number
                -> 1 + 2 - 1        // 这里直接循环吃token，就全部推导出来了，在exp结构体中就需要使用vector来容纳

    exp
   /   \
  1     + 2, - 1


直接从左往右扫描数组，解决了结合律问题


-----------------------------------


S -> exp
exp -> exp operator number | number
operator -> + | - | * | /


1 + 2 - 1
    -> S
        -> exp
            -> exp - 1
                -> exp + 2 - 1
                    -> 1 + 2 - 1


最终语法树：

          -
        /   \
       +     1
     /   \
    1     2

不存在歧义，要求exp展开后，末尾number的就是终结符，这样子也使其成为左结合，但是不满足乘除优先于加减的要求

!!!!尝试消除左递归，可以先跳过!!!!
和上面完全一致

-----------------------------------

S -> addexp
addexp -> addexp oper1 addexp | mulexp
oper1 -> + | -
mulexp -> mulexp oper2 number | number
oper2 -> * | /


1 + 2 * 3
    -> S
        -> addexp
            -> addexp + addexp
                -> mulexp + mulexp
                    -> 1 + mulexp * 3
                        -> 1 + 2 * 3

最终语法树：
          +
        /   \
       1     *
           /   \
          2     3


!!!!尝试消除左递归，可以先跳过!!!!
addexp 产生的句型必然是 mulexp { oper1 mulexp }

mulexp 产生的句型必然是 number { oper2 number }

S -> addexp
addexp -> mulexp addexp'
addexp' -> oper1 mulexp addexp' | ε
oper1 -> + | -
mulexp -> number mulexp'
mulexp' -> oper2 number mulexp' | ε
oper2 -> * | /

1 + 2 * 3
    -> S
        -> addexp
            -> mulexp addexp'
                -> mulexp + mulexp addexp'      -> mulexp + mulexp
                    -> 1 mulexp' + mulexp
                        -> 1 + 2 mulexp'
                            -> 1 + 2 * 3 mulexp'
                                -> 1 + 2 * 3
                
1 * 2 + 3
    -> S
        -> addexp
            -> mulexp addexp'
                -> mulexp + mulexp addexp'      -> mulexp + mulexp
                    -> 1 mulexp' + mulexp
                        -> 1 * 2 mulexp' + mulexp
                            -> 1 * 2 + mulexp
                                -> 1 * 2 + 3 mulexp'
                                    -> 1 * 2 + 3


抽象语法树：
          +
        /   \
       *     3
     /   \
    1     2





1 - 2 + 3
    -> S
        -> addexp
            -> mulexp addexp'
                -> mulexp - mulexp addexp'
                    -> mulexp - mulexp + mulexp addexp'
                        -> mulexp - mulexp + mulexp
                            -> 1 mulexp' - 2 mulexp' + 3 mulexp'
                                -> 1 - 2 + 3



抽象语法树：
          -
        /   \
       1     +
           /   \
          1     2


存在结合律问题


用EBNF可以描述成

S -> addexp
addexp -> mulexp { oper1 mulexp }
oper1 -> + | -
mulexp -> number { oper2 number }
oper2 -> * | /


1 + 2 * 1
    -> S
        -> addexp
            -> mulexp + mulexp
                -> number + number * number
                    -> 1 + 2 * 1


1 + 2 * 1 - 3 / 2 + 5
    -> S
        -> addexp
            -> mulexp + mulexp - mulexp + mulexp
                -> 1 +  2 * 1  -  3 / 2  + 5


抽象语法树：
                addexp
               /      \
            mulexp    +, mulexp,        -, mulexp,      +, mulexp
           /      \     /      \          /      \        /      \
          1       null 2       *, 1      3        /, 2   5        null

解决右结合律问题
-----------------------------------

上面的还不支持括号，接下来试着改一下

S -> addexp
addexp -> addexp oper1 addexp | mulexp
oper1 -> + | -
mulexp -> mulexp oper2 atomicexp | atomicexp
oper2 -> * | /
atomicexp -> '(' addexp ')' | number 


(1 + 2) * 3
    -> S
        -> addexp
            -> mulexp       因为无法满足addexp oper1 addexp的条件，"(1"不可能被addexp推导出来，括号会使其变成一个整体，在最后由atomicexp推导并去除括号
                -> mulexp * atomicexp
                    -> atomicexp * 3
                        -> ( addexp ) * 3
                            -> ( addexp + addexp ) * 3
                                -> ( mulexp + mulexp ) * 3
                                    -> ( atomicexp + atomicexp ) * 3
                                        -> ( 1 + 2 ) * 3

最终语法树：

          *
        /   \
       +     3
     /   \
    1     2

-----------------------------------

虽然支持括号，但是实现时，存在左递归的问题

接下来想办法修改文法消除左递归


S -> addexp
addexp -> addexp oper1 addexp | mulexp        // 左递归
oper1 -> + | -
mulexp -> mulexp oper2 atomicexp | atomicexp      // 左递归
oper2 -> * | /
atomicexp -> '(' addexp ')' | number





addexp 产生的句型一定是 mulexp { oper1 mulexp }
mulexp 产生的句型一定是 atomicexp { oper2 atomicexp }

S -> addexp
addexp -> mulexp { oper1 mulexp }
oper1 -> + | -
mulexp -> atomicexp { oper2 atomicexp }
oper2 -> * | /
atomicexp -> '(' addexp ')' | number





1 + 3 * (8 - 6 + 7) + 3 / 5
    -> S
        -> addexp
            -> mulexp + mulexp + mulexp
                -> 1 + atomicexp * atomicexp + atomicexp / 5


*/


#include "compiler/parser.h"

class Calculation {
    compiler::S * m_s;
public:
    Calculation(compiler::S* s) : m_s(s) { }


public:
    int CalculationAtomicExp(compiler::AtomicExp* atomicexp) {
        if (atomicexp->addexp == nullptr) {
            return atomicexp->num;
        }
        else {
            return CalculationAddExp(atomicexp->addexp.get());
        }
    }
    int CalculationMulExp(compiler::MulExp* mulexp) {
        int res = CalculationAtomicExp(mulexp->atomicexp.get());

        for (int i = 0; i < mulexp->oper2Arr.size(); i++) {
            if (mulexp->oper2Arr[i] == compiler::TokenType::mul) {
                res *= CalculationAtomicExp(mulexp->atomicexpArr[i].get());;
            }
            else {
                res /= CalculationAtomicExp(mulexp->atomicexpArr[i].get());;
            }
        }
        return res;
    }

    int CalculationAddExp(compiler::AddExp* addexp) {
        int res = CalculationMulExp(addexp->mulexp.get());
        for (int i = 0; i < addexp->oper1Arr.size(); i++) {
            if (addexp->oper1Arr[i] == compiler::TokenType::add) {
                res += CalculationMulExp(addexp->mulexpArr[i].get());;
            }
            else {
                res -= CalculationMulExp(addexp->mulexpArr[i].get());;
            }
        }
        return res;
    }


    int CalculationS() {
        return CalculationAddExp(m_s->addexp.get());
    }
};




int main()
{
    using namespace compiler;
    Lexer lexer("1 + 3 * (8 + 3 * (10 - 7)) + 3 / 5");
    Parser parser(&lexer);
    auto s = parser.ParseS();

    Calculation calc(s.get());

    int res = calc.CalculationS();

    std::cout << "Hello World!\n";
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
