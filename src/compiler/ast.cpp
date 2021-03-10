#include "./ast.h"

namespace lilang
{
    namespace ast
    {
        void File::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void Ident::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void BinaryExpr::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void UnaryExpr::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void BasicLiteral::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void ParenExpr::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void CallExpr::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void IndexExpr::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void StarExpr::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void ArrayType::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void FuncType::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void FuncLit::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void VarDecl::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void FuncDecl::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void IfStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void ForStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void WhileStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void Block::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void RetStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void EmptyStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void ExprStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void AssignStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }

        void DeclStmt::Accept(Visitor *v)
        {
            v->Visit(this);
        }
    }
}