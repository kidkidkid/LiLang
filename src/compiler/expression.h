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

        // Expr op Expr
        class BinaryExpr : public Expr
        {
        public:
            ExprType left;
            CodeType op;
            ExprType right;
            BinaryExpr() = default;
            BinaryExpr(ExprType l, CodeType t, ExprType r) : left(l), op(t), right(r) {}
            inline TokenPos Start()
            {
                return left->Start();
            }
        };

        // op Expr
        class UnaryExpr : public Expr
        {
        public:
            TokenPos pos;
            CodeType op;
            ExprType expr;
            UnaryExpr() = default;
            UnaryExpr(TokenPos p, CodeType t, ExprType e) : pos(p), op(t), expr(e) {}
            inline TokenPos Start()
            {
                return pos;
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
            ArrayType() = default;
            ArrayType(TokenPos l, ExprType e) : l(l), expr(e) {}
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

        // Expr(Expr)
        class CallExpr : public Expr
        {
        public:
            ExprType expr;
            ExprListType args;
            CallExpr() = default;
            CallExpr(ExprType f, ExprListType a) : expr(f), args(a) {}
            inline TokenPos Start()
            {
                return expr->Start();
            }
        };

        // operand[expr]
        class IndexExpr : public Expr
        {
        public:
            ExprType operand;
            ExprType index;
            IndexExpr() = default;
            IndexExpr(ExprType o, ExprType i) : operand(o), index(i) {}
            inline TokenPos Start()
            {
                return operand->Start();
            }
        };

    }
}

#endif
