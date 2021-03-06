#ifndef LILANG_AST_NODE
#define LILANG_AST_NODE

#include <vector>
#include <memory>
#include "../listl.h"
#include "./lexical.h"

#define lilang_trace

namespace lilang
{
    namespace ast
    {
        // typedef
        using TokenPos = int;
        enum class TypeKind
        {
            kInt,
            kFloat,
            kString,
            kFn,
            kArray,
            kPointer,
            kInvalid,
            // todo, data
        };
        class Obj;
        class Type
        {
        public:
            typedef std::shared_ptr<Type> Ptr;
            typedef std::vector<Ptr> List;

            TypeKind kind;
            // pointer OR array
            Ptr base;
            // function
            std::vector<std::shared_ptr<Obj>> params;
            List returns;

            Type() = default;
            Type(TypeKind k) : kind(k) {}
            Type(TypeKind k, Ptr b) : kind(k), base(b) {}
            Type(TypeKind k, std::vector<std::shared_ptr<Obj>> args, List rets)
                : kind(k), params(args), returns(rets) {}
            static bool match(Ptr t1, Ptr t2);
        };
        class Node
        {
        public:
            virtual ~Node(){};
        };

        //start symbol
        class File : public Node
        {
        };

        class Decl : public Node
        {
        public:
            typedef std::shared_ptr<Decl> Ptr;
            typedef std::vector<Ptr> List;
        };

        class Stmt : public Node
        {
        public:
            typedef std::shared_ptr<Stmt> Ptr;
            typedef std::vector<Ptr> List;
        };

        class Expr : public Node
        {
        public:
            typedef std::shared_ptr<Expr> Ptr;
            typedef std::vector<Ptr> List;

            Type::Ptr extract_type();
        };

        // named identity
        class Obj
        {
        public:
            typedef std::shared_ptr<Obj> Ptr;
            typedef std::vector<Ptr> List;

            string_t name;
            Type::Ptr type;

            //function
            Expr::Ptr func_lit;

            Obj() = default;
            Obj(Type::Ptr t, string_t n) : type(t), name(n) {}
        };

        //********************************************************************
        //expression related
        //********************************************************************

        class BadExpr : public Expr
        {
        public:
            BadExpr() = default;
        };

        class OperandName : public Expr
        {
        public:
            string_t name;
            OperandName() = default;
            OperandName(string_t n) : name(n) {}
        };

        // Expr op Expr
        class BinaryExpr : public Expr
        {
        public:
            Expr::Ptr left;
            compiler::CodeType op;
            Expr::Ptr right;
            BinaryExpr() = default;
            BinaryExpr(Expr::Ptr l, compiler::CodeType t, Expr::Ptr r) : left(l), op(t), right(r) {}
        };

        // op Expr
        class UnaryExpr : public Expr
        {
        public:
            compiler::CodeType op;
            Expr::Ptr expr;
            UnaryExpr() = default;
            UnaryExpr(compiler::CodeType t, Expr::Ptr e) : op(t), expr(e) {}
        };

        // literal: string/number/float
        class BasicLiteral : public Expr
        {
        public:
            Type::Ptr type;
            string_t value;
            BasicLiteral() = default;
            BasicLiteral(Type::Ptr t, string_t v) : type(t), value(v) {}
        };

        class FuncLit : public Expr
        {
        public:
            Type::Ptr signature;
            Stmt::Ptr block;
            FuncLit() = default;
            FuncLit(Type::Ptr fn, Stmt::Ptr b) : signature(fn), block(b) {}
        };

        // (expr)
        class ParenExpr : public Expr
        {
        public:
            Expr::Ptr expr;
            ParenExpr() = default;
            ParenExpr(Expr::Ptr e) : expr(e) {}
        };

        // expr(expr)
        class CallExpr : public Expr
        {
        public:
            Expr::Ptr expr;
            Expr::List args;
            CallExpr() = default;
            CallExpr(Expr::Ptr f, Expr::List a) : expr(f), args(a) {}
        };

        // type(expr)
        class CastExpr : public Expr
        {
        public:
            Type::Ptr type;
            Expr::Ptr expr;
            CastExpr() = default;
            CastExpr(Type::Ptr t, Expr::Ptr e) : type(t), expr(e) {}
        };

        // operand[expr]
        class IndexExpr : public Expr
        {
        public:
            Expr::Ptr operand;
            Expr::Ptr index;
            IndexExpr() = default;
            IndexExpr(Expr::Ptr o, Expr::Ptr i) : operand(o), index(i) {}
        };

        //********************************************************************
        // declaration related
        //********************************************************************

        // let x, y int
        // let x, y = 10, 10
        class VarDecl : public Decl
        {
        public:
            Obj::List names; // identifiers
            Type::Ptr type;
            Expr::List vals;
            VarDecl() = default;
            VarDecl(Obj::List n, Expr::List v) : names(n), vals(v) {}
            VarDecl(Obj::List n, Type::Ptr t) : names(n), type(t) {}
        };

        //********************************************************************
        // statement related
        //********************************************************************

        class IfStmt : public Stmt
        {
        public:
            Expr::Ptr condition;
            Stmt::Ptr if_block;
            Stmt::Ptr else_block;
            IfStmt() = default;
            IfStmt(Expr::Ptr cond, Stmt::Ptr if_block, Stmt::Ptr else_block)
                : condition(cond), if_block(if_block), else_block(else_block) {}
        };

        class WhileStmt : public Stmt
        {
        public:
            Expr::Ptr condition;
            Stmt::Ptr block;
            WhileStmt() = default;
            WhileStmt(Expr::Ptr cond, Stmt::Ptr block)
                : condition(cond), block(block) {}
        };

        class ForStmt : public Stmt
        {
        public:
            Stmt::Ptr init;
            Expr::Ptr condition;
            Stmt::Ptr post;
            Stmt::Ptr block;
            ForStmt() = default;
            ForStmt(Stmt::Ptr i, Expr::Ptr c, Stmt::Ptr p, Stmt::Ptr b)
                : init(i), condition(c), post(p), block(b) {}
        };

        class RetStmt : public Stmt
        {
        public:
            Expr::List vals;
            RetStmt() = default;
            RetStmt(Expr::List v) : vals(v) {}
        };

        class Block : public Stmt
        {
        public:
            Stmt::List stmts;
            Block() = default;
            Block(Stmt::List stmts) : stmts(stmts) {}
        };

        class EmptyStmt : public Stmt
        {
        public:
            EmptyStmt() = default;
        };

        class ExprStmt : public Stmt
        {
        public:
            Expr::Ptr expr;
            ExprStmt() = default;
            ExprStmt(Expr::Ptr e) : expr(e) {}
        };

        class AssignStmt : public Stmt
        {
        public:
            Expr::List lhs;
            Expr::List rhs;
            AssignStmt() = default;
            AssignStmt(Expr::List l, Expr::List r) : lhs(l), rhs(r) {}
        };

        class DeclStmt : public Stmt
        {
        public:
            Decl::Ptr decl;
            DeclStmt() = default;
            DeclStmt(Decl::Ptr d) : decl(d) {}
        };

    }
}

#endif