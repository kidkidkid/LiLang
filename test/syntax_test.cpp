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

string_t func_type = "fn (fn(int xx , int x)(), int x)(int, fn(int x)int)";

int main()
{
    CodeError::List err_list;
    auto tok_list = CodeFile::Parse(func_type, err_list);
    Parser parser(tok_list);
    for (auto iter : tok_list)
    {
        std::cout << iter.value << std::endl;
    }
    parser.parseType();
    parser.printErrors();
}
