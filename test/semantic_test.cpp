#define private public

#include "../src/compiler/syntax.h"
#include "../src/compiler/semantic.h"

using namespace lilang;
using namespace lilang::compiler;

int main()
{
    string_t f = "./example/testcode.li";
    Parser parser;
    auto root = parser.ParseFile(f);
    parser.PrintErrors();
    ast::AnalyzeSemantically(root);
}