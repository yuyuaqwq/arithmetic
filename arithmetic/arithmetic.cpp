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
private:
    std::string m_src;
    size_t m_idx;
    Token m_nextToken;

public:
    Lexer(std::string src) : m_src(src), m_idx(0) {
        m_nextToken.type = TokenType::none;
    }

public:
    char NextChar() {
        if (m_idx < m_src.size()) {
            return m_src[m_idx++];
        }
        return 0;
    }

    void SkipChar(int c) {
        m_idx += c;
    }

    Token LookAhead() {
        if (m_nextToken.type == TokenType::none) {
            m_nextToken = NextToken();
        }
        return m_nextToken;

    }

    Token NextToken() {
        
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

    void MatchToken(TokenType type) {
        Token token = NextToken();
        if (token.type != type) {
            throw std::exception("cannot match token");
        }
        return;
    }
};

class Parser {

    Lexer* m_lexer;
public:

    struct AddExp;

    struct AtomicExp {
        AddExp* addexp;
        int num;

        AtomicExp(AddExp* _addexp, int _num) : num(_num), addexp(_addexp) { }
    };

    struct MulExp {
        AtomicExp* atomicexp;
        std::vector<TokenType> oper2Arr;
        std::vector<AtomicExp*> atomicexpArr;

        MulExp(AtomicExp* _atomicexp, const std::vector<TokenType>& _oper2Arr, const std::vector<AtomicExp*>& _atomicexpArr) : atomicexp(_atomicexp), oper2Arr(_oper2Arr), atomicexpArr(_atomicexpArr) { }
    };



    struct AddExp {
        MulExp* mulexp;
        std::vector<TokenType> oper1Arr;
        std::vector<MulExp*> mulexpArr;
        AddExp(MulExp* _mulexp, const std::vector<TokenType>& _oper1Arr, const std::vector<MulExp*>& _mulexpArr) : mulexp(_mulexp), oper1Arr(_oper1Arr), mulexpArr(_mulexpArr) { }
    };

    struct S {
        AddExp* addexp;
        S(AddExp* _addexp) :addexp(_addexp) { }
    };


public:
    Parser(Lexer* lexer) : m_lexer(lexer) { }

public:

    S* ParseS() {
        return new S(ParseAddExp());
    }

    AddExp* ParseAddExp() {
        MulExp* mulexp = ParseMulExp();
        std::vector<TokenType> oper1Arr;
        std::vector<MulExp*> mulexpArr;

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


        return new AddExp(mulexp, oper1Arr, mulexpArr);
    }

    MulExp* ParseMulExp() {
        AtomicExp* atomicexp = ParseAtomicExp();
        std::vector<TokenType> oper2Arr;
        std::vector<AtomicExp*> atomicexpArr;

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


        return new MulExp(atomicexp, oper2Arr, atomicexpArr);
    }

    AtomicExp* ParseAtomicExp() {
        Token token = m_lexer->LookAhead();
        if (token.type == TokenType::eof) {
            throw std::exception("parse atomic exp error");
        }
        AddExp* addexp = nullptr;
        int num = 0;
        if (token.type == TokenType::lparen) {
            m_lexer->NextToken();
            addexp = ParseAddExp();
            m_lexer->MatchToken(TokenType::rparen);
        }
        else {
            num = ParseNumber();
        }
        return new AtomicExp(addexp, num);
    }


    int ParseNumber() {
        Token token = m_lexer->NextToken();
        if (token.type != TokenType::num) {
            throw std::exception("parse number error");
        }
        int num = atoi(token.str.c_str());
        return num;
    }


    //ExpTail* ParseExpTail() {
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



};

class Calculation {
    Parser::S* m_s;
public:
    Calculation(Parser::S* s) : m_s(s) { }


public:
    int CalculationAtomicExp(Parser::AtomicExp* atomicexp) {
        if (atomicexp->addexp == nullptr) {
            return atomicexp->num;
        }
        else {
            return CalculationAddExp(atomicexp->addexp);
        }
    }
    int CalculationMulExp(Parser::MulExp* mulexp) {
        int res = CalculationAtomicExp(mulexp->atomicexp);

        for (int i = 0; i < mulexp->oper2Arr.size(); i++) {
            if (mulexp->oper2Arr[i] == TokenType::mul) {
                res *= CalculationAtomicExp(mulexp->atomicexpArr[i]);;
            }
            else {
                res /= CalculationAtomicExp(mulexp->atomicexpArr[i]);;
            }
        }
        return res;
    }

    int CalculationAddExp(Parser::AddExp* addexp) {
        int res = CalculationMulExp(addexp->mulexp);
        for (int i = 0; i < addexp->oper1Arr.size(); i++) {
            if (addexp->oper1Arr[i] == TokenType::add) {
                res += CalculationMulExp(addexp->mulexpArr[i]);;
            }
            else {
                res -= CalculationMulExp(addexp->mulexpArr[i]);;
            }
        }
        return res;
    }


    int CalculationS() {
        return CalculationAddExp(m_s->addexp);
    }
};



int main()
{
    Lexer lexer("1 + 3 * (8 + 3 * (10 - 7)) + 3 / 5");

    Parser parser(&lexer);
    Parser::S* s = parser.ParseS();

    Calculation calc(s);

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
