#include <iostream>
#define private public
#include "../src/compiler/lexical.h"
#include "../src/compiler/syntax.h"

using namespace lilang::compiler;
using namespace lilang;

string_t syntax_code =
    R"(
int x = 1;
int tmp = 0;
for (int y = 1; y < 4; y += 1) 
{
    tmp += x * y; //注释
} 
return tmp;
)";

string_t func_type = "fn (fn(**int xx , int x)(), int x)(int, fn(int x)[]int)";
string_t unary_expr = "+-&^ffff(10, 10, 10)";
string_t binary_expr = "10 > (20 * 10) || 10 /10 < 100 || true && -100 > 0";

int main()
{
    CodeError::List err_list;
    auto tok_list = CodeFile::Parse(binary_expr, err_list);
    Parser parser(tok_list);
    for (auto iter : tok_list)
    {
        std::cout << iter.value << std::endl;
    }
    auto e = parser.parseExpression();
    parser.printErrors();
    auto c = std::dynamic_pointer_cast<ast::BinaryExpr>(e);
    std::cout << CodeToken::Type2Str(c->op) << std::endl << c->right->Start() << std::endl;
}
