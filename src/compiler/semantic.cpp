#include <iostream>
#include "./semantic.h"

namespace lilang
{
    namespace ast
    {
        void SemanticVisitor::Visit(File *f)
        {
            for (auto decl : f->declarations)
            {
                decl->Accept(this);
            }
        }
        void SemanticVisitor::Visit(Ident *)
        {
        }
        void SemanticVisitor::Visit(BinaryExpr *)
        {
        }
        void SemanticVisitor::Visit(UnaryExpr *)
        {
        }
        void SemanticVisitor::Visit(BasicLiteral *)
        {
        }
        void SemanticVisitor::Visit(ParenExpr *)
        {
        }
        void SemanticVisitor::Visit(CallExpr *)
        {
        }
        void SemanticVisitor::Visit(IndexExpr *)
        {
        }
        void SemanticVisitor::Visit(StarExpr *)
        {
        }
        void SemanticVisitor::Visit(ArrayType *)
        {
        }
        void SemanticVisitor::Visit(FuncType *)
        {
        }
        void SemanticVisitor::Visit(FuncLit *)
        {
        }
        void SemanticVisitor::Visit(VarDecl *decl)
        {
            std::cout << decl->names[0] << std::endl;
        }

        void SemanticVisitor::Visit(FuncDecl *decl)
        {
            std::cout << decl->fn_name << std::endl;
        }
        void SemanticVisitor::Visit(IfStmt *)
        {
        }
        void SemanticVisitor::Visit(WhileStmt *)
        {
        }
        void SemanticVisitor::Visit(ForStmt *)
        {
        }
        void SemanticVisitor::Visit(AssignStmt *)
        {
        }
        void SemanticVisitor::Visit(DeclStmt *)
        {
        }
        void SemanticVisitor::Visit(RetStmt *)
        {
        }
        void SemanticVisitor::Visit(Block *)
        {
        }
        void SemanticVisitor::Visit(ExprStmt *)
        {
        }
        void SemanticVisitor::Visit(EmptyStmt *)
        {
        }

        void AnalyzeSemantically(Node::Ptr node)
        {
            auto visitor = SemanticVisitor();
            node->Accept(&visitor);
        }
    }
}