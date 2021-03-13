#include <iostream>
#include "./ast.h"

namespace lilang
{
    namespace ast
    {

        bool Type::Match(const Type::Ptr &t1, const Type::Ptr &t2)
        {
            if (t1 == nullptr || t2 == nullptr)
            {
                return false;
            }
            // skip invalid, errors may have beed emitted before
            // if (t1->kind == Type::Kind::kInvalid || t2->kind == Type::Kind::kInvalid)
            // {
            //     return true;
            // }
            if (t1->kind != t2->kind)
            {
                return false;
            }
            if (t1->kind == Type::Kind::kArray)
            {
                return Match(t1->base, t2->base);
            }
            if (t1->kind == Type::Kind::kPointer)
            {
                return Match(t1->base, t2->base);
            }
            if (t1->kind == Type::Kind::kFn)
            {
                if (t1->params.size() != t2->params.size() || t1->returns.size() != t2->returns.size())
                {
                    return false;
                }
                for (int i = 0; i < t1->params.size(); i++)
                {
                    if (!Match(t1->params[i], t2->params[i]))
                    {
                        return false;
                    }
                }
                for (int i = 0; i < t1->returns.size(); i++)
                {
                    if (!Match(t1->returns[i], t2->returns[i]))
                    {
                        return false;
                    }
                }
            }
            if (t1->kind == Type::Kind::kTuple)
            {
                for (int i = 0; i < t1->vals.size(); i++)
                {
                    if (!Match(t1->vals[i], t2->vals[2]))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        bool Type::Comparable(const Ptr &t)
        {
            if (t->kind == Kind::kInt ||
                t->kind == Kind::kFloat ||
                t->kind == Kind::kBool)
            {
                return true;
            }
            return false;
        }

        bool Type::CanCast(const Ptr &from, const Ptr &to)
        {
            if (Match(from, to))
            {
                return true;
            }
            // int -> float
            if (from->kind == Type::Kind::kInt && to->kind == Type::Kind::kFloat)
            {
                return true;
            }
            // float -> int
            if (from->kind == Type::Kind::kFloat && to->kind == Type::Kind::kInt)
            {
                return true;
            }
            return false;
        }

        string_t Type::String(const Ptr &t)
        {
            if (t == nullptr)
            {
                return "";
            }
            switch (t->kind)
            {
            case Type::Kind::kInt:
                return "int";
            case Type::Kind::kFloat:
                return "float";
            case Type::Kind::kBool:
                return "bool";
            case Type::Kind::kString:
                return "string";
            case Type::Kind::kFn:
            {
                stringstream_t ss;
                ss << "fn(";
                for (auto arg : t->params)
                {
                    ss << String(arg) << ",";
                }
                ss << ")(";
                for (auto ret : t->returns)
                {
                    ss << String(ret) << ",";
                }
                ss << ")";
                return ss.str();
            }
            case Type::Kind::kPointer:
                return "*" + String(t->base);
            case Type::Kind::kArray:
                return "[]" + String(t->base);
            case Type::Kind::kTuple:
            {
                stringstream_t ss;
                ss << "(";
                for (auto tt : t->vals)
                {
                    ss << String(tt) << ",";
                }
                ss << ")";
                return ss.str();
            }
            case Type::Kind::kInvalid:
                return "invalid";
            default:
                exit(-1);
            }
        }

        bool Obj::Addressable()
        {
            if (kind == Kind::kVar ||
                kind == Kind::kIndexValue)
            {
                return true;
            }
            return false;
        }

        bool Obj::Assignable()
        {
            if (kind == Kind::kVar ||
                kind == Kind::kIndexValue ||
                kind == Kind::kIndirectPointer)
            {
                return true;
            }
            return false;
        }

        string_t Obj::String()
        {
            auto kind2String = [](Kind k) -> string_t {
                switch (k)
                {
                case Kind::kType:
                    return "type";
                case Kind::kVar:
                    return "var";
                case Kind::kFunc:
                    return "func";
                case Kind::kValue:
                    return "value";
                case Kind::kIndexValue:
                    return "index_value";
                case Kind::kIndirectPointer:
                    return "indirect_pointer";
                default:
                    exit(-1);
                }
            };
            stringstream_t ss;
            ss << "Kind: " << kind2String(kind) << " "
               << "Type: " << Type::String(type);
            return ss.str();
        }

        Obj::Ptr Obj::InvalidInstance()
        {
            static Ptr o = std::make_shared<Obj>(Obj::Kind::kValue, std::make_shared<Type>(Type::Kind::kInvalid));
            return o;
        }

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