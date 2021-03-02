#ifndef LILANG_AST_NODE
#define LILANG_AST_NODE

#include "../listl.h"

#define lilang_trace

namespace lilang
{
    namespace ast
    {
        class Expr;
        class Stmt;
        class Decl;
        class Field;
        // typedef
        using TokenPos = int;
        using ExprType = std::shared_ptr<Expr>;
        using ExprListType = std::vector<ExprType>;
        using FieldType = std::shared_ptr<Field>;
        using FieldListType = std::vector<FieldType>;
        using StmtType = std::shared_ptr<Stmt>;
        using StmtListType = std::vector<StmtType>;
        using DeclType = std::shared_ptr<Decl>;
        using DeclListType = std::vector<DeclType>;

        class Node
        {
        public:
            virtual ~Node(){};
            virtual TokenPos Start() = 0;
        };

        //start symbol
        class File : public Node
        {
        };

        class Decl : public Node
        {
        };

        class Stmt : public Node
        {
        };

        class Expr : public Node
        {
        };

        // {identifier} type
        class Field : public Node
        {
        public:
            ExprType name; // identifier
            ExprType type;
            Field() = default;
            Field(ExprType t) : type(t) {}
            Field(ExprType t, ExprType n) : name(n), type(t) {}
            inline TokenPos Start()
            {
                return type->Start();
            }
        };

    }
}

#endif