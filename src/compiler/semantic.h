#ifndef LILANG_COMPILER_SEMANTIC
#define LILANG_COMPILER_SEMANTIC

#include <map>
#include "./ast.h"

namespace lilang
{
    namespace ast
    {
        class Scope
        {
        public:
            typedef std::shared_ptr<Scope> Ptr;

            Ptr parent;
            Scope() = default;
            Scope(Ptr parent) : parent(parent) {}
            void AddSymbol(const string_t &, Obj::Ptr);
            Obj::Ptr FindSymbol(const string_t &);
            bool IsSymBolDeclared(const string_t &); // in this scope

            static int Depth;

        private:
            std::map<string_t, Obj::Ptr> symbol_table;
        };

        class SemanticVisitor : public Visitor
        {
        public:
            SemanticVisitor();
            void Analyze(Node::Ptr node);
            void PrintErrors();

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
            void Visit(VarDecl *) override;  // decl here
            void Visit(FuncDecl *) override; /// decl here
            void Visit(IfStmt *) override;
            void Visit(WhileStmt *) override;
            void Visit(ForStmt *) override;
            void Visit(AssignStmt *) override;
            void Visit(DeclStmt *) override; //decl here
            void Visit(RetStmt *) override;
            void Visit(Block *) override;
            void Visit(ExprStmt *) override;
            void Visit(EmptyStmt *) override;

        private:
            Scope::Ptr scope;
            void EnterScope();
            void LeaveScope();
            void EmitError(const string_t &);

            class Error
            {
            public:
                string_t msg;
                Error() = default;
                Error(const string_t &s) : msg(s) {}
            };
            std::vector<Error> errors;
        };
    }
}

#endif