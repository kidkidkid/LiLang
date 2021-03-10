#ifndef LILANG_COMPILER_SEMANTIC
#define LILANG_COMPILER_SEMANTIC

#include "./ast.h"

namespace lilang
{
    namespace ast
    {

        enum class TypeKind
        {
            kInt,
            kFloat,
            kString,
            kFn,
            kArray,
            kPointer,
        };

        enum class ObjKind
        {
            kVar,
            kFunc,
            kType,
        };

        class Type
        {
        public:
            typedef std::shared_ptr<Type> Ptr;
            typedef std::vector<Ptr> List;

            TypeKind kind;
            // pointer OR array
            Ptr base;
            // function
            List params;
            List returns;

            Type() = default;
            Type(TypeKind k) : kind(k) {}
            Type(TypeKind k, Ptr b) : kind(k), base(b) {}
            Type(TypeKind k, List args, List rets)
                : kind(k), params(args), returns(rets) {}
            static bool match(Ptr t1, Ptr t2);
        };

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

        class Scope
        {
        public:
        };

        class SemanticVisitor : public Visitor
        {
        public:
            void Visit(File *) override;
            void Visit(Ident *) override;
            void Visit(BinaryExpr *) override;
            void Visit(UnaryExpr *) override;
            void Visit(BasicLiteral *) override;
            void Visit(ParenExpr *) override;
            void Visit(CallExpr *) override;
            void Visit(IndexExpr *) override;
            void Visit(StarExpr *) override;
            void Visit(ArrayType *) override;
            void Visit(FuncType *) override;
            void Visit(FuncLit *) override;
            void Visit(VarDecl *) override;
            void Visit(FuncDecl *) override;
            void Visit(IfStmt *) override;
            void Visit(WhileStmt *) override;
            void Visit(ForStmt *) override;
            void Visit(AssignStmt *) override;
            void Visit(DeclStmt *) override;
            void Visit(RetStmt *) override;
            void Visit(Block *) override;
            void Visit(ExprStmt *) override;
            void Visit(EmptyStmt *) override;
        };

        void AnalyzeSemantically(Node::Ptr node);
    }
}

#endif