#include <iostream>
#define private public
#include "../src/compiler/lexical.h"
#include "../src/compiler/syntax.h"

using namespace lilang::compiler;
using namespace lilang;

string_t func_type = "fn (fn(**int xx , []*int x)(), int x)(int, fn(int x)[]int)";
string_t unary_expr = "+-&^ffff(10, 10, 10)";
string_t binary_expr = "10 > (20 * 10) || 10 /10 < 100 || true && -100 > 0";
string_t cast_expr = "int8(100 + 100)";
string_t expr_list = "10 < 100 && 10 > 1, x <= 100, 10 >= z";
string_t simple_stmt = "x, y, z = 10, 10, 10";
string_t block_stmt =
    R"({
        x, y, z = 10;
        x = 1000;
        z = "123123";
        mm = 10.001;
})";
string_t let_stmt = "let x, y, z = 10 + 10, name(10), 0x123123;";
string_t ret_stmt = "return 10, 10+10, y, name(10, 10, 10*100+10);";
string_t if_stmt_one =
    R"(
    if (x < 1000 && 100 > 10) {
        x = 10;
    }
)";
string_t if_stmt_two =
    R"(
        if (100 > 10 || 1 == 10) 
        {
            x = 10;
        } else {
            y = 100;
        }
)";
string_t if_stmt_three =
    R"(
    
        if (100 > 10 || 1 == 10) 
        {
            x = 10;
        } else if (true){
            y = 100;
        } else  {
            y = 1000;
        }
)";
string_t for_stmt =
    R"(
    for(let x = 1; x < 100; x += 10) {
        let x, y, z int;
        x, y, z = 100, 10, 1;
    }
)";
string_t while_stmt =
    R"(
    while(x <= 10 || x >= 100) {
        x, y := 10, 100;
    }
)";

int main()
{
    int x;
    CodeError::List err_list;
    auto tok_list = CodeFile::Parse(while_stmt, err_list);
    for (auto err : err_list)
    {
        std::cout << err.error_msg << std::endl;
    }
    Parser parser(tok_list);
    auto e = parser.parseStmt();
    parser.printErrors();
    // auto c = std::dynamic_pointer_cast<ast::BinaryExpr>(e);
    // std::cout << CodeToken::Type2Str(c->op) << std::endl
    //           << c->right->Start() << std::endl;
}
