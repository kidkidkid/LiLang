#include "../src/compiler/syntax.h"

using namespace lilang;
using namespace lilang::compiler;

int main()
{
    string_t f = "./example/testcode.li";
    Parser parser;
    parser.ParseFile(f);
    parser.PrintErrors();
}