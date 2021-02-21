#include "../src/compiler/lexical.h"
#include <iostream>

using namespace lilang;
using namespace lilang::compiler;

string_t comment_code =
    R"(
//comment test one
 //comment test two
    //comment test three  //nested
//中文注释)";

string_t string_literal =
    R"("abcdefg\
asd"   "abcde\t\\" "asds)";

string_t number_code =
    R"(0 0.1 1.0 0xasdf 0b10101 0b122 0o7)";

string_t identifier_code =
    R"(a82n_ad sadf_q3 4545n_t
    sdf_fd____)";

string_t operator_code =
    R"(+-*/>=<=+=-+/=/={}(),;)";

string_t compound_code =
    R"(
int x = 1;
int tmp = 0;
for (int y = 1; y < 4; y += 1) 
{
    tmp += x * y; //注释
} 
return tmp;
)";

void printTokens(CodeToken::List &list)
{
    std::cout << "TOKENS" << std::endl;
    for (auto token : list)
    {
        std::cout << "(" << token.row_number << ", " << token.column_number << ") "
                  << CodeToken::Type2Str(token.type) << " " << token.token << std::endl;
    }
}

void printErrors(CodeError::List &list)
{
    std::cout << "ERRORS" << std::endl;
    for (auto err : list)
    {
        std::cout << "(" << err.row_number << ", " << err.column_number << ") "
                  << err.error_msg << std::endl;
    }
}

int main()
{
    CodeError::List err_list;
    auto tok_list = CodeFile::Parse(compound_code, err_list);
    printTokens(tok_list);
    printErrors(err_list);
}
