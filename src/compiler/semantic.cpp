#include <iostream>
#include "./semantic.h"

#define lilang_semantic_trace

#define trace(S)                                \
    do                                          \
    {                                           \
        RepeatStringLit(Scope::Depth * 4, "."); \
        RepeatStringLit(1, S);                  \
        RepeatStringLit(1, "\n");               \
    } while (0)

namespace lilang
{
    namespace ast
    {
        int Scope::Depth = 0;

        void Scope::AddSymbol(const string_t &name, Obj::Ptr obj)
        {
#ifdef lilang_semantic_trace
            stringstream_t ss;
            ss << "Add Symbol " << name << " " << obj->String();
            trace(ss.str());
#endif
            this->symbol_table[name] = obj;
        }

        Obj::Ptr Scope::FindSymbol(const string_t &name)
        {
            auto val = symbol_table[name];
            if (val != nullptr)
            {
                return val;
            }
            auto s = this->parent;
            while (s)
            {
                auto val = s->symbol_table[name];
                if (val != nullptr)
                {
                    return val;
                }
                s = s->parent;
            }
            return nullptr;
        }

        bool Scope::IsSymBolDeclared(const string_t &name)
        {
            auto v = symbol_table[name];
            if (v == nullptr)
            {
                return false;
            }
            return true;
        }

        SemanticVisitor::SemanticVisitor()
        {
            scope = std::make_shared<Scope>();
            // built-in symbols
            auto type_int = std::make_shared<Type>(Type::Kind::kInt);
            auto type_float = std::make_shared<Type>(Type::Kind::kFloat);
            auto type_string = std::make_shared<Type>(Type::Kind::kString);
            scope->AddSymbol("int", std::make_shared<Obj>(Obj::Kind::kType, type_int));
            scope->AddSymbol("float", std::make_shared<Obj>(Obj::Kind::kType, type_float));
            scope->AddSymbol("string", std::make_shared<Obj>(Obj::Kind::kType, type_string));
        }

        void SemanticVisitor::EnterScope()
        {
            auto s = std::make_shared<Scope>();
            s->parent = scope;
            scope = s;
            Scope::Depth++;
        }

        void SemanticVisitor::LeaveScope()
        {
            scope = scope->parent;
            Scope::Depth--;
        }

        void SemanticVisitor::EmitError(const string_t &msg)
        {
            RepeatStringLit(Scope::Depth * 4, ".");
            RepeatStringLit(1, msg);
            RepeatStringLit(1, "\n");
            errors.emplace_back(msg);
        }

        void SemanticVisitor::Analyze(Node::Ptr node)
        {
            node->Accept(this);
        }

        void SemanticVisitor::AnalyzeExprList(Expr::List &list)
        {
            for (auto node : list)
            {
                node->Accept(this);
            }
        }

        void SemanticVisitor::AnalyzeStmtList(Stmt::List &list)
        {
            for (auto node : list)
            {
                node->Accept(this);
            }
        }

        void SemanticVisitor::PrintErrors()
        {
            for (auto err : errors)
            {
                std::cout << err.msg << std::endl;
            }
        }

        void SemanticVisitor::Visit(File *f)
        {
            for (auto decl : f->declarations)
            {
                decl->Accept(this);
            }
        }

        //********************************************************************
        // expression related
        //********************************************************************

        // should be declared
        // may be wrong if A.B is a valid grammer
        void SemanticVisitor::Visit(Ident *ident)
        {
            auto o = scope->FindSymbol(ident->name);
            if (o == nullptr)
            {
                EmitError(ident->name + " is not declared before");
                ident->obj = Obj::InvalidInstance();
                return;
            }
            ident->obj = o;
        }

        void SemanticVisitor::Visit(BinaryExpr *binary_expr)
        {
            binary_expr->obj = Obj::InvalidInstance();
            auto lhs = binary_expr->left;
            auto rhs = binary_expr->right;
            Analyze(lhs);
            Analyze(rhs);
            switch (binary_expr->op)
            {
            // both sides are bool
            case compiler::CodeType::kLogicAnd:
            case compiler::CodeType::kLogicOr:
            {
                if (lhs->obj->type->kind == Type::Kind::kBool && rhs->obj->type->kind == Type::Kind::kBool)
                {
                    binary_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, lhs->obj->type);
                }
                else
                {
                    EmitError("both side of ||/&& should be type bool");
                }
                return;
            }
            // both sides have the same type and comparable
            case compiler::CodeType::kEqual:
            case compiler::CodeType::kNotEqual:
            case compiler::CodeType::kLess:
            case compiler::CodeType::kGreater:
            case compiler::CodeType::kNotGreater:
            case compiler::CodeType::kNotLess:
            {
                if (Type::CouldAssign(lhs->obj->type, rhs->obj->type) && Type::Comparable(lhs->obj->type))
                {
                    auto t = std::make_shared<Type>(Type::Kind::kBool);
                    binary_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, t);
                }
                else
                {
                    EmitError("both sides of ==/!=/</>/<=/>= should have the same type and comparable");
                }
                return;
            }
            // both side are int or float
            case compiler::CodeType::kAdd:
            case compiler::CodeType::kSub:
            case compiler::CodeType::kMultiply:
            case compiler::CodeType::kDivide:
            {
                if (lhs->obj->type->kind != Type::Kind::kInt && lhs->obj->type->kind != Type::Kind::kFloat ||
                    rhs->obj->type->kind != Type::Kind::kInt && rhs->obj->type->kind != Type::Kind::kFloat)
                {
                    EmitError("both sides of +/-/*// should be type int ot float");
                }
                else
                {
                    // todo, float???
                    auto t = std::make_shared<Type>(Type::Kind::kFloat);
                    binary_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, t);
                }
                return;
            }
            // both side are int
            case compiler::CodeType::kBitsOr:
            case compiler::CodeType::kBitsXor:
            case compiler::CodeType::kBitsAnd:
            case compiler::CodeType::kMod:
            {
                if (lhs->obj->type->kind == Type::Kind::kInt && rhs->obj->type->kind == Type::Kind::kInt)
                {
                    auto t = std::make_shared<Type>(Type::Kind::kInt);
                    binary_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, t);
                }
                else
                {
                    EmitError("both sides of |/&/^/% should be type int");
                }
                return;
            }
            default:
                exit(-1);
            }
        }

        void SemanticVisitor::Visit(UnaryExpr *unary_expr)
        {
            auto expr = unary_expr->expr;
            Analyze(expr);
            switch (unary_expr->op)
            {
            case compiler::CodeType::kAdd:
            case compiler::CodeType::kSub:
            case compiler::CodeType::kBitsXor:
            {
                if (expr->obj->type->kind != Type::Kind::kInt)
                {
                    EmitError("+/-/^ expects a number operand");
                    unary_expr->obj = Obj::InvalidInstance();
                }
                else
                {
                    auto t = std::make_shared<Type>(Type::Kind::kInt);
                    unary_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, t);
                }
            }
            break;
            case compiler::CodeType::kLogicNot:
            {
                if (expr->obj->type->kind != Type::Kind::kBool)
                {
                    EmitError("! expects a bool operand");
                    unary_expr->obj = Obj::InvalidInstance();
                }
                else
                {
                    auto t = std::make_shared<Type>(Type::Kind::kBool);
                    unary_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, t);
                }
            }
            break;
            case compiler::CodeType::kBitsAnd:
            {
                if (!expr->obj->Addressable())
                {
                    EmitError("& expects an addressble operand");
                    unary_expr->obj = Obj::InvalidInstance();
                }
                else
                {
                    auto t = std::make_shared<Type>(Type::Kind::kPointer, expr->obj->type);
                    unary_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, t);
                }
            }
            break;
            default:
                exit(-1);
            }
        }

        void SemanticVisitor::Visit(BasicLiteral *lit)
        {
            auto obj = std::make_shared<Obj>(Obj::Kind::kValue, nullptr);
            switch (lit->type)
            {
            case compiler::CodeType::kNumber:
                obj->type = std::make_shared<Type>(Type::Kind::kInt);
                break;
            case compiler::CodeType::kFloat:
                obj->type = std::make_shared<Type>(Type::Kind::kFloat);
                break;
            case compiler::CodeType::kStringLiteral:
                obj->type = std::make_shared<Type>(Type::Kind::kString);
                break;
            default:
                exit(-1);
            }
            lit->obj = obj;
        }

        void SemanticVisitor::Visit(ParenExpr *paren_expr)
        {
            Analyze(paren_expr->expr);
            paren_expr->obj = paren_expr->expr->obj;
        }

        void SemanticVisitor::Visit(CallExpr *call_expr)
        {
            call_expr->obj = Obj::InvalidInstance();
            auto expr = call_expr->expr;
            Analyze(expr);
            AnalyzeExprList(call_expr->args);
            // type cast
            if (expr->obj->kind == Obj::Kind::kType)
            {
                if (call_expr->args.size() != 1)
                {
                    EmitError("type cast expect one operand");
                    return;
                }
                auto operand = call_expr->args[0];
                if (!Type::CouldAssign(operand->obj->type, expr->obj->type))
                {
                    stringstream_t ss;
                    ss << "Cannot cast from " << Type::String(operand->obj->type)
                       << " to " << Type::String(expr->obj->type);
                    EmitError(ss.str());
                    return;
                }
                call_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, expr->obj->type);
                return;
            }
            // variable of function OR function lit
            else if (expr->obj->kind == Obj::Kind::kVar && expr->obj->type->kind == Type::Kind::kFn ||
                     expr->obj->kind == Obj::Kind::kFunc)
            {
                // skip
            }
            else
            {
                EmitError("type or function expected, found type " + Type::String(expr->obj->type));
                return;
            }
            auto args = expr->obj->type->params;
            if (args.size() != call_expr->args.size())
            {
                auto call_args = call_expr->args;
                if (call_args.size() == 1 &&
                    call_args[0]->obj->type->kind == Type::Kind::kTuple &&
                    call_args[0]->obj->type->vals.size() == args.size())
                {
                    if (args.size() == 0) {
                        EmitError("function returns nothing, cannot used as a value");
                        return;
                    }
                    for (int i = 0; i < args.size(); i++)
                    {
                        if (!Type::CouldAssign(call_args[0]->obj->type->vals[i], args[i]))
                        {
                            stringstream_t ss;
                            ss << "Cannot pass type " << Type::String(call_args[0]->obj->type->vals[i])
                               << " to type " << Type::String(args[i]);
                            EmitError(ss.str());
                            return;
                        }
                    }
                }
                else
                {
                    EmitError("call arguments number not equals to that of function arguments");
                    return;
                }
            }
            else
            {
                for (int i = 0; i < args.size(); i++)
                {
                    auto call_arg = call_expr->args[i];
                    if (!Type::CouldAssign(call_arg->obj->type, args[i]))
                    {
                        stringstream_t ss;
                        ss << "function argument expects types " << Type::String(args[i])
                           << " passed type " << Type::String(call_arg->obj->type);
                        EmitError(ss.str());
                        return;
                    }
                }
            }
            // function returns a tuple
            Type::Ptr t;
            if (expr->obj->type->returns.size() == 1)
            {
                t = expr->obj->type->returns[0];
            }
            else
            {
                t = std::make_shared<Type>(Type::Kind::kTuple, expr->obj->type->returns);
            }
            call_expr->obj = std::make_shared<Obj>(Obj::Kind::kValue, t);
        }

        void SemanticVisitor::Visit(IndexExpr *index_expr)
        {
            index_expr->obj = Obj::InvalidInstance();
            auto operand = index_expr->operand;
            auto index = index_expr->index;
            Analyze(operand);
            Analyze(index);
            if (operand->obj->type->kind != Type::Kind::kArray)
            {
                EmitError("index operand must be array-type");
                return;
            }
            if (index->obj->type->kind != Type::Kind::kInt)
            {
                EmitError("array index must be a number");
                return;
            }
            index_expr->obj = std::make_shared<Obj>(Obj::Kind::kIndexValue, operand->obj->type->base);
        }

        void SemanticVisitor::Visit(StarExpr *star_expr)
        {
            auto expr = star_expr->expr;
            Analyze(expr);
            // pointer type
            if (expr->obj->kind == Obj::Kind::kType)
            {
                auto t = std::make_shared<Type>(Type::Kind::kPointer, expr->obj->type);
                star_expr->obj = std::make_shared<Obj>(Obj::Kind::kType, t);
            }
            // dereference
            else if (expr->obj->type->kind == Type::Kind::kPointer)
            {
                star_expr->obj = std::make_shared<Obj>(Obj::Kind::kIndirectPointer, expr->obj->type->base);
            }
            else
            {
                EmitError("type or pointer variable expected");
                star_expr->obj = Obj::InvalidInstance();
            }
        }

        void SemanticVisitor::Visit(ArrayType *array_type)
        {
            auto expr = array_type->expr;
            Analyze(expr);
            if (expr->obj->kind != Obj::Kind::kType)
            {
                EmitError("[] expects a type");
                array_type->obj = Obj::InvalidInstance();
            }
            else
            {
                auto t = std::make_shared<Type>(Type::Kind::kArray, expr->obj->type);
                array_type->obj = std::make_shared<Obj>(Obj::Kind::kType, t);
            }
        }

        void SemanticVisitor::Visit(FuncType *func_type)
        {
            Type::List params;
            Type::List returns;
            for (auto param : func_type->args)
            {
                Analyze(param->type);
                if (param->type->obj->kind != Obj::Kind::kType)
                {
                    EmitError("function argument type is not valid");
                    return;
                }
                else
                {
                    params.push_back(param->type->obj->type);
                }
            }
            for (auto ret : func_type->returns)
            {
                Analyze(ret);
                if (ret->obj->kind != Obj::Kind::kType)
                {
                    EmitError("function return type is not valid");
                    return;
                }
                else
                {
                    returns.push_back(ret->obj->type);
                }
            }
            auto t = std::make_shared<Type>(Type::Kind::kFn, params, returns);
            func_type->obj = std::make_shared<Obj>(Obj::Kind::kType, t);
        }

        void SemanticVisitor::Visit(FuncLit *lit)
        {
            Analyze(lit->type);
            lit->obj = std::make_shared<Obj>(Obj::Kind::kFunc, lit->type->obj->type);
            if (lit->name != "")
            {
                scope->AddSymbol(lit->name, lit->obj);
            }
            EnterScope();
            for (auto arg : lit->type->args)
            {
                if (arg->var_name != "_")
                {
                    if (scope->IsSymBolDeclared(arg->var_name))
                    {
                        EmitError("duplicate arguments in function");
                        return;
                    }
                    else
                    {
                        scope->AddSymbol(arg->var_name, arg->type->obj);
                    }
                }
            }
            AnalyzeStmtList(lit->body->stmts);
            LeaveScope();
        }

        //********************************************************************
        // declaration related
        //********************************************************************

        void SemanticVisitor::Visit(VarDecl *decl)
        {
            for (auto name : decl->names)
            {
                if (scope->IsSymBolDeclared(name))
                {
                    EmitError("variable " + name + " is redeclared");
                    return;
                }
            }
            if (decl->type != nullptr)
            {
                Analyze(decl->type);
                for (auto name : decl->names)
                {
                    auto o = std::make_shared<Obj>(Obj::Kind::kVar, decl->type->obj->type);
                    scope->AddSymbol(name, o);
                }
            }
            else
            {
                AnalyzeExprList(decl->vals);
                if (decl->names.size() != decl->vals.size())
                {
                    // let x, y, z = func()
                    if (decl->vals.size() == 1 &&
                        decl->vals[0]->obj->type->kind == Type::Kind::kTuple &&
                        decl->vals[0]->obj->type->vals.size() == decl->names.size())
                    {
                        for (int i = 0; i < decl->names.size(); i++)
                        {
                            auto o = std::make_shared<Obj>(Obj::Kind::kVar, decl->vals[0]->obj->type->vals[i]);
                            scope->AddSymbol(decl->names[i], o);
                        }
                        return;
                    }
                    else
                    {
                        EmitError("the number of right expression mismatch the variable number");
                        return;
                    }
                }
                // let x, y, z = 10, 10, 10
                // let x, y, z = func(), 10, 10
                for (int i = 0; i < decl->names.size(); i++)
                {
                    auto expr = decl->vals[i];
                    if (expr->obj->type->kind == Type::Kind::kTuple && expr->obj->type->vals.size() != 1)
                    {
                        EmitError("tuple" + Type::String(expr->obj->type) +
                                  " cannot assign to a single variable");
                        return;
                    }
                    else if (expr->obj->type->kind == Type::Kind::kTuple)
                    {
                        auto o = std::make_shared<Obj>(Obj::Kind::kVar, expr->obj->type->vals[0]);
                        scope->AddSymbol(decl->names[i], o);
                        continue;
                    }
                    auto o = std::make_shared<Obj>(Obj::Kind::kVar, expr->obj->type);
                    scope->AddSymbol(decl->names[i], o);
                }
            }
        }

        void SemanticVisitor::Visit(FuncDecl *decl)
        {
            if (scope->IsSymBolDeclared(decl->fn_lit->name))
            {
                EmitError("function " + decl->fn_lit->name + " is redeclared");
                return;
            }
            Analyze(decl->fn_lit); // func type
        }

        //********************************************************************
        // statement related
        //********************************************************************

        void SemanticVisitor::Visit(IfStmt *)
        {
        }
        void SemanticVisitor::Visit(WhileStmt *)
        {
        }
        void SemanticVisitor::Visit(ForStmt *)
        {
        }

        void SemanticVisitor::Visit(AssignStmt *assign_stmt)
        {
            auto lhs = assign_stmt->lhs;
            auto rhs = assign_stmt->rhs;
            AnalyzeExprList(lhs);
            AnalyzeExprList(rhs);
            if (lhs.size() != rhs.size())
            {
                // let x, y, z = func()
                if (rhs.size() == 1 &&
                    rhs[0]->obj->type->kind == Type::Kind::kTuple &&
                    rhs[0]->obj->type->vals.size() == lhs.size())
                {
                    for (int i = 0; i < lhs.size(); i++)
                    {
                        auto left_type = lhs[i]->obj->type;
                        auto right_type = rhs[0]->obj->type->vals[i];
                        if (!Type::CouldAssign(right_type, left_type))
                        {
                            stringstream_t ss;
                            ss << "Cannot assign type " << Type::String(right_type)
                               << " to type " << Type::String(left_type);
                            EmitError(ss.str());
                            return;
                        }
                    }
                    return;
                }
                else
                {
                    EmitError("the number of right side mismatch the left side");
                    return;
                }
            }
            else
            {
                for (int i = 0; i < lhs.size(); i++)
                {
                    auto left_type = lhs[i]->obj->type;
                    auto right_type = rhs[i]->obj->type;
                    if (!Type::CouldAssign(right_type, left_type))
                    {
                        stringstream_t ss;
                        ss << "Cannot assign type " << Type::String(right_type)
                           << " to type " << Type::String(left_type);
                        EmitError(ss.str());
                        return;
                    }
                }
            }
        }

        void SemanticVisitor::Visit(DeclStmt *decl_stmt)
        {
            Analyze(decl_stmt->decl);
        }

        void SemanticVisitor::Visit(RetStmt *)
        {
        }

        void SemanticVisitor::Visit(Block *block)
        {
            EnterScope();
            AnalyzeStmtList(block->stmts);
            LeaveScope();
        }

        void SemanticVisitor::Visit(ExprStmt *expr_stmt)
        {
            auto expr = expr_stmt->expr;
            Analyze(expr);
        }

        void SemanticVisitor::Visit(EmptyStmt *)
        {
        }

    }
}