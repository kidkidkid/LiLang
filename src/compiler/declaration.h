#ifndef LILANG_COMPILER_DECLARATION
#define LILANG_COMPILER_DECLARATION

#include "./ast.h"
#include "./expression.h"
#include "./statement.h"

namespace lilang
{
    namespace ast
    {
        class Ident;
        class FuncType;
        class Block;

        class FuncDecl : public Decl
        {
        public:
            TokenPos fn;
            std::shared_ptr<Ident> name;         // identifier
            std::shared_ptr<FuncType> func_type; // funcType
            std::shared_ptr<Block> block;
            FuncDecl() = default;
            FuncDecl(TokenPos f, std::shared_ptr<Ident> n, std::shared_ptr<FuncType> t,
                     std::shared_ptr<Block> b) : fn(f), name(n), func_type(t), block(b) {}
            inline TokenPos Start()
            {
                return fn;
            }
        };

        class ValDecl : public Decl
        {
        public:
            TokenPos let;
            ExprListType names; // identifiers
            ExprType type;
            ExprListType vals;
            ValDecl() = default;
            ValDecl(TokenPos l, ExprListType n, ExprListType v)
                : let(l), names(n), vals(v) {}
            ValDecl(TokenPos l, ExprListType n, ExprType t) : let(l), names(n), type(t) {}
            inline TokenPos Start()
            {
                return let;
            }
        };

    }
}

#endif