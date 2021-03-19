#ifndef LILANG_AST_NODE
#define LILANG_AST_NODE

#include <vector>
#include <memory>
#include "../listl.h"
#include "./lexical.h"

#define RepeatStringLit(N, S)   \
    for (int i = 0; i < N; i++) \
    {                           \
        std::cout << S;         \
    }

namespace lilang
{
    namespace ast
    {
        using TokenPos = int;
        class Visitor;

        //********************************************************************
        // ast node attribute
        //********************************************************************

        class Type
        {
        public:
            typedef std::shared_ptr<Type> Ptr;
            typedef std::vector<Ptr> List;

            enum class Kind
            {
                kInt,
                kFloat,
                kString,
                kBool,
                kFn,
                kArray,
                kPointer,
                kTuple,
                kInvalid,
            };

            Kind kind;
            // pointer OR array
            Ptr base;
            // function
            List params;
            List returns;
            // tuple
            List vals;

            Type() = default;
            Type(Kind k) : kind(k) {}
            Type(Kind k, Ptr b) : kind(k), base(b) {}
            Type(Kind k, List args, List rets) : kind(k), params(args), returns(rets) {}
            Type(Kind k, List vals) : kind(k), vals(vals) {}

            static bool Match(const Ptr &, const Ptr &);
            static bool Comparable(const Ptr &);
            static string_t String(const Ptr &);
            static bool CouldAssign(const Ptr &, const Ptr &);
        };

        class Obj
        {
        public:
            typedef std::shared_ptr<Obj> Ptr;
            typedef std::vector<Ptr> List;

            // Variable/IndexValue/IndirectPointer can be left value
            enum class Kind
            {
                kVar,             // variable
                kFunc,            // function lit
                kType,            // purely type
                kValue,           // computed value, including literal
                kIndexValue,      // array index
                kIndirectPointer, // *
            };

            Kind kind;
            Type::Ptr type;

            Obj() = default;
            Obj(Kind k, Type::Ptr t) : type(t), kind(k) {}

            bool Addressable();
            bool Assignable();
            string_t String();
            static Ptr InvalidInstance();
        };

        //********************************************************************
        // ast node related
        //********************************************************************

        class Node
        {
        public:
            typedef std::shared_ptr<Node> Ptr;
            typedef std::vector<Ptr> List;

            virtual void Accept(Visitor *v) = 0;
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

            virtual bool HasTerminating() = 0;
        };

        class Expr : public Node
        {
        public:
            Obj::Ptr obj;

            typedef std::shared_ptr<Expr> Ptr;
            typedef std::vector<Ptr> List;
        };

        class Field
        {
        public:
            typedef std::shared_ptr<Field> Ptr;
            typedef std::vector<Ptr> List;

            string_t var_name;
            Expr::Ptr type;
            Field() = default;
            Field(string_t n, Expr::Ptr t) : var_name(n), type(t) {}
        };

        //********************************************************************
        //  start symbol
        //********************************************************************

        class File : public Node
        {
        public:
            typedef std::shared_ptr<File> Ptr;

            Decl::List declarations;
            File() = default;
            inline void AddDecl(Decl::Ptr d)
            {
                declarations.push_back(std::move(d));
            }
            void Accept(Visitor *);
        };

        //********************************************************************
        // expression related
        //********************************************************************

        class BadExpr : public Expr
        {
        public:
            BadExpr() = default;
            void Accept(Visitor *v);
        };

        class Ident : public Expr
        {
        public:
            string_t name;
            Ident() = default;
            Ident(string_t n) : name(n) {}
            void Accept(Visitor *v);
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
            void Accept(Visitor *v);
        };

        // op Expr
        class UnaryExpr : public Expr
        {
        public:
            compiler::CodeType op;
            Expr::Ptr expr;
            UnaryExpr() = default;
            UnaryExpr(compiler::CodeType t, Expr::Ptr e) : op(t), expr(e) {}
            void Accept(Visitor *v);
        };

        // literal: string/number/float
        class BasicLiteral : public Expr
        {
        public:
            string_t value;
            compiler::CodeType type;
            BasicLiteral() = default;
            BasicLiteral(string_t v, compiler::CodeType t) : value(v), type(t) {}
            void Accept(Visitor *v);
        };

        // (expr)
        class ParenExpr : public Expr
        {
        public:
            Expr::Ptr expr;
            ParenExpr() = default;
            ParenExpr(Expr::Ptr e) : expr(e) {}
            void Accept(Visitor *v);
        };

        // expr(expr)
        class CallExpr : public Expr
        {
        public:
            Expr::Ptr expr;
            Expr::List args;
            CallExpr() = default;
            CallExpr(Expr::Ptr f) : expr(f) {}
            CallExpr(Expr::Ptr f, Expr::List a) : expr(f), args(a) {}
            void Accept(Visitor *v);
        };

        // operand[expr]
        class IndexExpr : public Expr
        {
        public:
            Expr::Ptr operand;
            Expr::Ptr index;
            IndexExpr() = default;
            IndexExpr(Expr::Ptr o, Expr::Ptr i) : operand(o), index(i) {}
            void Accept(Visitor *v);
        };

        class StarExpr : public Expr
        {
        public:
            Expr::Ptr expr;
            StarExpr() = default;
            StarExpr(Expr::Ptr e) : expr(e) {}
            void Accept(Visitor *v);
        };

        class ArrayType : public Expr
        {
        public:
            Expr::Ptr expr;
            ArrayType() = default;
            ArrayType(Expr::Ptr e) : expr(e) {}
            void Accept(Visitor *v);
        };

        class FuncType : public Expr
        {
        public:
            typedef std::shared_ptr<FuncType> Ptr;

            Field::List args;
            Expr::List returns;
            FuncType() = default;
            FuncType(Field::List args, Expr::List rets) : args(args), returns(rets) {}
            void Accept(Visitor *v);
        };

        class Block;
        class FuncLit : public Expr
        {
        public:
            typedef std::shared_ptr<FuncLit> Ptr;

            string_t name;
            FuncType::Ptr type;
            std::shared_ptr<Block> body;
            FuncLit() = default;
            FuncLit(FuncType::Ptr t, std::shared_ptr<Block> b) : type(t), body(b) {}
            void Accept(Visitor *v);
        };

        //********************************************************************
        // declaration related
        //********************************************************************

        // let x, y int
        // let x, y = 10, 10
        class VarDecl : public Decl
        {
        public:
            std::vector<string_t> names;
            Expr::Ptr type;
            Expr::List vals;
            VarDecl() = default;
            VarDecl(std::vector<string_t> n, Expr::Ptr t) : names(n), type(t) {}
            VarDecl(std::vector<string_t> n, Expr::List vals) : names(n), vals(vals) {}
            void Accept(Visitor *v);
        };

        // fn name()()
        class FuncDecl : public Decl
        {
        public:
            FuncLit::Ptr fn_lit;
            FuncDecl() = default;
            FuncDecl(FuncLit::Ptr f) : fn_lit(f) {}
            void Accept(Visitor *v);
        };

        //********************************************************************
        // statement related
        //********************************************************************

        class Block : public Stmt
        {
        public:
            typedef std::shared_ptr<Block> Ptr;

            Stmt::List stmts;
            Block() = default;
            Block(Stmt::List stmts) : stmts(stmts) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class BadStmt : public Stmt
        {
        public:
            BadStmt() = default;
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class IfStmt : public Stmt
        {
        public:
            Expr::Ptr condition;
            Stmt::Ptr if_block;
            Stmt::Ptr else_block;
            IfStmt() = default;
            IfStmt(Expr::Ptr cond, Stmt::Ptr if_block, Stmt::Ptr else_block)
                : condition(cond), if_block(if_block), else_block(else_block) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class WhileStmt : public Stmt
        {
        public:
            Expr::Ptr condition;
            Stmt::Ptr block;
            WhileStmt() = default;
            WhileStmt(Expr::Ptr cond, Stmt::Ptr block)
                : condition(cond), block(block) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class ForStmt : public Stmt
        {
        public:
            Stmt::Ptr init;
            Expr::Ptr condition;
            Stmt::Ptr post;
            Block::Ptr block;
            ForStmt() = default;
            ForStmt(Stmt::Ptr i, Expr::Ptr c, Stmt::Ptr p, Block::Ptr b)
                : init(i), condition(c), post(p), block(b) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class RetStmt : public Stmt
        {
        public:
            Expr::List vals;
            RetStmt() = default;
            RetStmt(Expr::List v) : vals(v) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class EmptyStmt : public Stmt
        {
        public:
            EmptyStmt() = default;
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class ExprStmt : public Stmt
        {
        public:
            Expr::Ptr expr;
            ExprStmt() = default;
            ExprStmt(Expr::Ptr e) : expr(e) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class AssignStmt : public Stmt
        {
        public:
            Expr::List lhs;
            Expr::List rhs;
            AssignStmt() = default;
            AssignStmt(Expr::List l, Expr::List r) : lhs(l), rhs(r) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class DeclStmt : public Stmt
        {
        public:
            Decl::Ptr decl;
            DeclStmt() = default;
            DeclStmt(Decl::Ptr d) : decl(d) {}
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class ContinueStmt : public Stmt
        {
        public:
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        class BreakStmt : public Stmt
        {
        public:
            void Accept(Visitor *v);
            bool HasTerminating();
        };

        //********************************************************************
        // visitor related
        //********************************************************************
        class Visitor
        {
        public:
            virtual void Visit(File *) = 0;
            virtual void Visit(Ident *) = 0;
            virtual void Visit(BinaryExpr *) = 0;
            virtual void Visit(UnaryExpr *) = 0;
            virtual void Visit(BasicLiteral *) = 0;
            virtual void Visit(ParenExpr *) = 0;
            virtual void Visit(CallExpr *) = 0;
            virtual void Visit(IndexExpr *) = 0;
            virtual void Visit(StarExpr *) = 0;
            virtual void Visit(ArrayType *) = 0;
            virtual void Visit(FuncType *) = 0;
            virtual void Visit(FuncLit *) = 0;
            virtual void Visit(VarDecl *) = 0;
            virtual void Visit(FuncDecl *) = 0;
            virtual void Visit(IfStmt *) = 0;
            virtual void Visit(WhileStmt *) = 0;
            virtual void Visit(ForStmt *) = 0;
            virtual void Visit(AssignStmt *) = 0;
            virtual void Visit(DeclStmt *) = 0;
            virtual void Visit(RetStmt *) = 0;
            virtual void Visit(Block *) = 0;
            virtual void Visit(ExprStmt *) = 0;
            virtual void Visit(EmptyStmt *) = 0;
            virtual void Visit(BadExpr *) = 0;
            virtual void Visit(BadStmt *) = 0;
            virtual void Visit(ContinueStmt *) = 0;
            virtual void Visit(BreakStmt *) = 0;
        };

    }
}

#endif