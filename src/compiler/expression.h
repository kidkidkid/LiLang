#ifndef LILANG_COMPILER_EXPRESSION
#define LILANG_COMPILER_EXPRESSION

#include <memory>
#include "./ast.h"
#include "./lexical.h"
#include "./statement.h"

namespace lilang
{
    namespace ast
    {
        class Expr;

        using namespace compiler;

        enum class IdentType
        {
            kType,
            kVariable,
            kFunc
        };

        // bad expression
        class BadExpr : public Expr
        {
        public:
            TokenPos l;
            TokenPos r;
            BadExpr() = default;
            BadExpr(TokenPos l, TokenPos r) : l(l), r(r) {}
            inline TokenPos Start()
            {
                return l;
            }
        };

        // literal: string/number/float
        class BasicLiteral : public Expr
        {
        public:
            CodeType kind;
            string_t value;
            TokenPos pos;
            BasicLiteral() = default;
            BasicLiteral(CodeType t, string_t v, TokenPos p) : kind(t), value(v), pos(p) {}
            inline TokenPos Start()
            {
                return pos;
            }
        };

        // identifier: type/variable/func
        class Ident : public Expr
        {
        public:
            IdentType kind;
            string_t name;
            TokenPos pos;
            Ident() = default;
            Ident(IdentType t, string_t n, TokenPos p) : kind(t), name(n), pos(p) {}
            inline TokenPos Start()
            {
                return pos;
            }
        };

        // fn(type {ident}{, type {ident}})[type{,type}]
        class FuncType : public Expr
        {
        public:
            TokenPos fn;
            FieldListType parameters;
            FieldListType returns;
            FuncType() = default;
            FuncType(TokenPos f, FieldListType p, FieldListType r) : fn(f), parameters(p), returns(r) {}
            inline TokenPos Start()
            {
                return fn;
            }
        };

        class FuncLit : public Expr
        {
        public:
            ExprType func_type;
            StmtType block;
            FuncLit() = default;
            FuncLit(ExprType fn, StmtType b) : func_type(fn), block(b) {}
            inline TokenPos Start()
            {
                return func_type->Start();
            }
        };

        // Expr(Expr)
        class CallExpr : public Expr
        {
        public:
            ExprType expr;
            TokenPos left_paren;
            ExprListType args;
            TokenPos right_paren;
            CallExpr() = default;
            CallExpr(ExprType f, TokenPos left, ExprListType a, TokenPos right)
                : expr(f), left_paren(left), args(a), right_paren(right) {}
            inline TokenPos Start()
            {
                return expr->Start();
            }
        };

        // *Expr
        class StarExpr : public Expr
        {
        public:
            TokenPos star;
            ExprType expr;
            StarExpr() = default;
            StarExpr(TokenPos p, ExprType e) : expr(e), star(p) {}
            inline TokenPos Start()
            {
                return star;
            }
        };

        // []expr
        class ArrayType : public Expr
        {
        public:
            TokenPos l;
            ExprType expr;
            TokenPos r;
            ArrayType() = default;
            ArrayType(TokenPos l, ExprType e, TokenPos r) : l(l), expr(e), r(r) {}
            inline TokenPos Start()
            {
                return l;
            }
        };

        // (expr)
        class ParenExpr : public Expr
        {
        public:
            TokenPos l;
            ExprType expr;
            TokenPos r;
            ParenExpr() = default;
            ParenExpr(TokenPos l, ExprType e, TokenPos r) : l(l), expr(e), r(r) {}
            inline TokenPos Start()
            {
                return l;
            }
        };
    }
}

#endif
