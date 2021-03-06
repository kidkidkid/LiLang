#include <sstream>
#include <iostream>
#include <iomanip>
#include "./syntax.h"

#define repeatStringLit(N, S)   \
    for (int i = 0; i < N; i++) \
    {                           \
        std::cout << S;         \
    }
#define trace(S) Trace _t(S, this)

using namespace lilang::compiler;
using namespace lilang;

Parser::TokenMap Parser::expression_follow = {
    {CodeType::kRightParenthese, true},
    {CodeType::kComma, true},
    {CodeType::kSemiColon, true},
    {CodeType::kRightBracket, true},
};
Parser::TokenMap Parser::statement_follow = {
    {CodeType::kRightBrace, true},
    {CodeType::kSemiColon, true},
};

Parser::Parser(CodeToken::List list)
{
    this->tokens = list;
    this->cur_iter = tokens.begin(); // 一开始用了list.begin()，不能取局部变量的迭代器
    this->cur_pos = 0;
    this->cur_tok = *cur_iter;
}

// helper
void Parser::nextToken()
{
#ifdef lilang_trace
    auto tok = tokens[cur_pos];
    std::cout << std::setw(3) << tok.row_number << ":" << std::setw(3) << tok.column_number << ":";
    repeatStringLit(Trace::trace_ident * 2, ".");
    std::cout << "\"" << cur_tok.value << "\"" << std::endl;
#endif
    if (cur_iter != tokens.end() && cur_tok.type != CodeType::kEOF)
    {
        cur_iter++;
        cur_pos++;
        cur_tok = *cur_iter;
    }
}

ast::TokenPos Parser::expect(CodeType t)
{
    if (cur_tok.type != t)
    {
        stringstream_t ss;
        ss << CodeToken::Type2Str(t) << " expected, found " << cur_tok.value;
        error_list.push_back({cur_pos, ss.str()});
    }
    nextToken();
    return cur_pos;
}

void Parser::expectError(const string_t &msg)
{
    stringstream_t ss;
    ss << msg << " expected, found " << cur_tok.value;
    error_list.push_back({cur_pos, ss.str()});
}

void Parser::exhaust(TokenMap &mp)
{
    while (cur_tok.type != CodeType::kEOF && mp[cur_tok.type] == false)
    {
        nextToken();
    }
}

int Parser::Trace::trace_ident = 0;
Parser::Trace::Trace(const string_t &msg, Parser *p) : p(p)
{
    auto tok = p->tokens[p->cur_pos];
    std::cout << std::setw(3) << tok.row_number << ":" << std::setw(3) << tok.column_number << ":";
    repeatStringLit(trace_ident * 2, ".");
    repeatStringLit(1, msg);
    repeatStringLit(1, "(\n");
    trace_ident++;
}

Parser::Trace::~Trace()
{
    trace_ident--;
    auto tok = p->tokens[p->cur_pos];
    std::cout << std::setw(3) << tok.row_number << ":" << std::setw(3) << tok.column_number << ":";
    repeatStringLit(trace_ident * 2, ".");
    repeatStringLit(1, ")\n");
}

void Parser::printErrors()
{
    if (error_list.size() == 0)
    {
        return;
    }
    std::cout << "Errors:" << std::endl;
    for (auto err : error_list)
    {
        std::cout << "(" << tokens[err.pos].row_number << ", " << tokens[err.pos].column_number << ")"
                  << err.msg << std::endl;
    }
}

//********************************************************************
//expression related
//********************************************************************

ast::Expr::Ptr Parser::parseExpression()
{
#ifdef lilang_trace
    trace("Expression");
#endif
    return parseBinaryExpression(1);
}

// expression{, expression}
ast::Expr::List Parser::parseExprList()
{
#ifdef lilang_trace
    trace("ExpressionList");
#endif
    ast::Expr::List list;
    while (true)
    {
        list.push_back(parseExpression()); // rvalue
        if (cur_tok.type != CodeType::kComma)
        {
            break;
        }
        expect(CodeType::kComma);
    }
    return list;
}

// The power of Recursive!
ast::Expr::Ptr Parser::parseBinaryExpression(int before_prec)
{
#ifdef lilang_trace
    trace("BinaryExpr");
#endif
    auto x = parseUnaryExpression();
    while (true)
    {
        int cur_prec = CodeToken::Precedence(cur_tok.type);
        auto op = cur_tok.type;
        // if the next token has lower precedence, the current expression
        // should evaluate itself firstly, and the next token would be
        // the root of the expression, current expression would be the left
        // child.
        // the first call's argument of parseBinaryExpression is 1, and nothing
        // would satisfy this condition until the end of expresssion
        if (cur_prec < before_prec)
        {
            return x;
        }
        expect(op);
        auto right = parseBinaryExpression(cur_prec + 1);
        x = std::make_shared<ast::BinaryExpr>(x, op, right);
    }
}

ast::Expr::Ptr Parser::parseUnaryExpression()
{
#ifdef lilang_trace
    trace("UnaryExpr");
#endif
    switch (cur_tok.type)
    {
    case CodeType::kAdd:
    case CodeType::kSub:
    case CodeType::kBitsAnd:
    case CodeType::kBitsXor:
    case CodeType::kBitsOr:
    case CodeType::kLogicNot:
    {
        auto t = cur_tok.type;
        nextToken();
        auto ue = parseUnaryExpression();
        return std::make_shared<ast::UnaryExpr>(t, ue);
    }
    default:
        return parsePrimaryExpression();
    }
}

// type(expression)
// operand[expression]
// operand(arg1, arg2, arg3)
ast::Expr::Ptr Parser::parsePrimaryExpression()
{
#ifdef lilang_trace
    trace("PrimaryExpr");
#endif
    auto p = parseOperand();
    while (true)
    {
        if (cur_tok.type == CodeType::kLeftParenthese)
        {
            p = parseCall(p);
        }
        else if (cur_tok.type == CodeType::kLeftBracket)
        {
            p = parseIndex(p);
        }
        else
        {
            break;
        }
    }
    return p;
}

ast::Expr::Ptr Parser::parseOperand()
{
#ifdef lilang_trace
    trace("Operand");
#endif
    switch (cur_tok.type)
    {
    case CodeType::kIdentifier: // variable
    {
        auto e = tryParseOperandName();
        if (e != nullptr)
        {
            return e;
        }
        break;
    }
    case CodeType::kLeftParenthese: // (expression)
    {
        expect(CodeType::kLeftParenthese);
        auto e = parseExpression();
        expect(CodeType::kRightParenthese);
        return std::make_shared<ast::ParenExpr>(e);
    }
    case CodeType::kStringLiteral:
    case CodeType::kNumber:
    case CodeType::kFloat:
        return parseBasicLit();
    case CodeType::kFn:
    {
        // function literal OR function type cast
        auto t = parseFuncType();
        if (cur_tok.type == CodeType::kLeftBrace)
        {
            return std::make_shared<ast::FuncLit>(t, parseBlock());
        }
        else
        {
            return parseCast(t);
        }
    }
    default:
        break;
    }
    // type cast
    auto t = tryParseType();
    if (t != nullptr)
    {
        return parseCast(t);
    }
    expectError("operand");
    exhaust(expression_follow);
    return std::make_shared<ast::BadExpr>();
}

ast::Expr::Ptr Parser::tryParseOperandName()
{
    if (cur_tok.type != CodeType::kIdentifier)
    {
        return nullptr;
    }
    //todo, type?
    if (cur_tok.value != "int" && cur_tok.value != "float" && cur_tok.value != "string")
    {
        auto o = std::make_shared<ast::OperandName>(cur_tok.value);
        nextToken();
        return o;
    }
    return nullptr;
}

ast::Expr::Ptr Parser::parseCall(ast::Expr::Ptr e)
{
#ifdef lilang_trace
    trace("Call");
#endif
    ast::Expr::List arg_list;
    expect(CodeType::kLeftParenthese);
    while (true)
    {
        arg_list.push_back(parseExpression());
        if (cur_tok.type == CodeType::kComma)
        {
            nextToken();
            continue;
        }
        else
        {
            break;
        }
    }
    expect(CodeType::kRightParenthese);
    return std::make_shared<ast::CallExpr>(e, arg_list);
}

ast::Expr::Ptr Parser::parseCast(ast::Type::Ptr t)
{
#ifdef lilang_trace
    trace("Cast");
#endif
    expect(CodeType::kLeftParenthese);
    auto e = parseExpression();
    expect(CodeType::kRightParenthese);
    return std::make_shared<ast::CastExpr>(t, e);
}

ast::Expr::Ptr Parser::parseIndex(ast::Expr::Ptr o)
{
#ifdef lilang_trace
    trace("Index");
#endif
    expect(CodeType::kLeftBracket);
    auto i = parseExpression();
    expect(CodeType::kRightBracket);
    return std::make_shared<ast::IndexExpr>(o, i);
}

// parse type
ast::Type::Ptr Parser::parseType()
{
    auto t = tryParseType();
    if (t == nullptr)
    {
        expectError("type");
        nextToken();
        return std::make_shared<ast::Type>(ast::TypeKind::kInvalid);
    }
    return t;
}

// return type or null if no type found
ast::Type::Ptr Parser::tryParseType()
{
    switch (cur_tok.type)
    {
    case CodeType::kMultiply:
        return parsePointerType();
    case CodeType::kLeftBracket:
        return parseArrayType();
    case CodeType::kIdentifier:
        return parseTypeName();
    case CodeType::kFn:
        return parseFuncType();
    default:
        return nullptr;
    }
}

// identifier typename
ast::Type::Ptr Parser::parseTypeName()
{
#ifdef lilang_trace
    trace("TypeName");
#endif
    ast::TypeKind kind;
    if (cur_tok.value == "int")
    {
        kind = ast::TypeKind::kInt;
    }
    else if (cur_tok.value == "string")
    {
        kind = ast::TypeKind::kString;
    }
    else if (cur_tok.value == "float")
    {
        kind = ast::TypeKind::kFloat;
    }
    else // todo typedef
    {
        return nullptr;
    }
    nextToken();
    return std::make_shared<ast::Type>(kind);
}

// ****int
ast::Type::Ptr Parser::parsePointerType()
{
#ifdef lilang_trace
    trace("PointerExpr");
#endif
    expect(CodeType::kMultiply);
    auto t = parseType();
    return std::make_shared<ast::Type>(ast::TypeKind::kPointer, t);
}

// [][][]*int
ast::Type::Ptr Parser::parseArrayType()
{
#ifdef lilang_trace
    trace("ArrayExpr");
#endif
    expect(CodeType::kLeftBracket);
    expect(CodeType::kRightBracket);
    auto t = parseType();
    return std::make_shared<ast::Type>(ast::TypeKind::kArray, t);
}

// fn(int, int)()
ast::Type::Ptr Parser::parseFuncType()
{
#ifdef lilang_trace
    trace("FuncType");
#endif
    expect(CodeType::kFn);
    auto params = parseFnParamters();
    auto returns = parseFnResults();
    return std::make_shared<ast::Type>(ast::TypeKind::kFn, params, returns);
}

// (int x , int y, float z)
// (int, float)
ast::Obj::List Parser::parseFnParamters()
{
#ifdef lilang_trace
    trace("Parametes");
#endif
    ast::Obj::List params;
    expect(CodeType::kLeftParenthese);
    if (cur_tok.type != CodeType::kRightParenthese)
    {
        while (true)
        {
            params.push_back(parseField());
            if (cur_tok.type == CodeType::kComma)
            {
                nextToken();
                continue;
            }
            else
            {
                break;
            }
        }
    }
    expect(CodeType::kRightParenthese);
    return params;
}

// int
// ()
// (int, int)
ast::Type::List Parser::parseFnResults()
{
#ifdef lilang_trace
    trace("Results");
#endif
    ast::Type::List returns;
    // have multiple retuen values
    if (cur_tok.type == CodeType::kLeftParenthese)
    {
        nextToken();
        if (cur_tok.type != CodeType::kRightParenthese)
        {
            while (true)
            {
                returns.push_back(parseType());
                if (cur_tok.type == CodeType::kComma)
                {
                    nextToken();
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        expect(CodeType::kRightParenthese);
    }
    else // have exactly one type or no return values
    {
        auto t = parseType();
        if (t != nullptr)
        {
            returns.push_back(t);
        }
    }
    return returns;
}

// type identifier OR type
ast::Obj::Ptr Parser::parseField()
{
    auto t = parseType();
    if (cur_tok.type == CodeType::kComma ||
        cur_tok.type == CodeType::kRightParenthese ||
        cur_tok.type == CodeType::kEOF)
    {
        return std::make_shared<ast::Obj>(t, "_");
    }
    nextToken();
    return std::make_shared<ast::Obj>(t, cur_tok.value);
}

// literal
ast::Expr::Ptr Parser::parseBasicLit()
{
    ast::Type::Ptr t;
    if (cur_tok.type == CodeType::kNumber)
    {
        t = std::make_shared<ast::Type>(ast::TypeKind::kInt);
    }
    else if (cur_tok.type == CodeType::kFloat)
    {
        t = std::make_shared<ast::Type>(ast::TypeKind::kFloat);
    }
    else if (cur_tok.type == CodeType::kStringLiteral)
    {
        t = std::make_shared<ast::Type>(ast::TypeKind::kString);
    }
    auto lit = std::make_shared<ast::BasicLiteral>(t, cur_tok.value);
    nextToken();
    return lit;
}

//********************************************************************
// statement related
//********************************************************************

// semicolon is expect in each statement, some statements dont end with semicolon
ast::Stmt::Ptr Parser::parseStmt()
{
#ifdef lilang_trace
    trace("Statement");
#endif
    switch (cur_tok.type)
    {
    case CodeType::kIf:
        return parseIfStmt();
    case CodeType::kWhile:
        return parseWhileStmt();
    case CodeType::kFor:
        return parseForStmt();
    case CodeType::kLeftBrace:
        return parseBlock();
    case CodeType::kReturn:
        return parseReturnStmt();
    case CodeType::kLet:
        return parseDeclStmt();
    case CodeType::kSemiColon:
    {
        nextToken();
        return std::make_shared<ast::EmptyStmt>();
    }
    // first of expression
    case CodeType::kIdentifier:
    case CodeType::kAdd:
    case CodeType::kSub:
    case CodeType::kBitsAnd:
    case CodeType::kBitsXor:
    case CodeType::kBitsOr:
    case CodeType::kLogicNot:
    case CodeType::kStringLiteral:
    case CodeType::kNumber:
    case CodeType::kFloat:
    case CodeType::kLeftParenthese:
        return parseSimpleStmt();
    default:
        break;
    };
    expectError("statement");
    exhaust(statement_follow); // if can not parse stmt, exhaust until ; OR }
    return std::make_shared<ast::EmptyStmt>();
}

ast::Stmt::List Parser::parseStmtList()
{
#ifdef lilang_trace
    trace("StatementList");
#endif
    ast::Stmt::List list;
    while (true)
    {
        if (cur_tok.type == CodeType::kEOF ||
            cur_tok.type == CodeType::kRightBrace)
        {
            break;
        }
        list.push_back(parseStmt()); // rvalue
    }
    return list;
}

// assign
ast::Stmt::Ptr Parser::parseSimpleStmt()
{
#ifdef lilang_trace
    trace("SimpleStatement");
#endif
    auto lhs = parseExprList();
    switch (cur_tok.type)
    {
    case CodeType::kAssign:
    case CodeType::kAddAssign:
    case CodeType::kSubAssign:
    case CodeType::kMulAssign:
    case CodeType::kDivAssign:
    case CodeType::kBitsOrAssign:
    case CodeType::kBitsAndAssign:
    case CodeType::kBitsXorAssign:
    {
        nextToken();
        auto rhs = parseExprList();
        expect(CodeType::kSemiColon);
        return std::make_shared<ast::AssignStmt>(lhs, rhs);
    }
    default:
        break;
    }
    if (lhs.size() > 1)
    {
        expectError("one expression");
        return std::make_shared<ast::EmptyStmt>();
    }
    // todo, maybe add ++/-- or other features
    expect(CodeType::kSemiColon);                   // end with semicolon
    return std::make_shared<ast::ExprStmt>(lhs[0]); // expression statement
}

ast::Stmt::Ptr Parser::parseIfStmt()
{
#ifdef lilang_trace
    trace("IfStatement");
#endif
    expect(CodeType::kIf);
    expect(CodeType::kLeftParenthese);
    auto cond = parseExpression();
    expect(CodeType::kRightParenthese);
    auto if_block = parseBlock();
    ast::Stmt::Ptr else_block = nullptr;
    if (cur_tok.type == CodeType::kElse)
    {
        nextToken();
        if (cur_tok.type == CodeType::kIf)
        {
            else_block = parseIfStmt();
        }
        else
        {
            else_block = parseBlock();
        }
    }
    return std::make_shared<ast::IfStmt>(cond, if_block, else_block);
}

ast::Stmt::Ptr Parser::parseWhileStmt()
{
#ifdef lilang_trace
    trace("WhileStatement");
#endif
    return nullptr;
}

ast::Stmt::Ptr Parser::parseForStmt()
{
#ifdef lilang_trace
    trace("IfStatement");
#endif
    return nullptr;
}

ast::Stmt::Ptr Parser::parseReturnStmt()
{
#ifdef lilang_trace
    trace("ReturnStatement");
#endif
    expect(CodeType::kReturn);
    auto rhs = parseExprList();
    expect(CodeType::kSemiColon);
    return std::make_shared<ast::RetStmt>(rhs);
}

// let x, y, z type;
// let x, y, z = e1, e2, e3;
ast::Stmt::Ptr Parser::parseDeclStmt()
{
#ifdef lilang_trace
    trace("DeclStatement");
#endif
    return std::make_shared<ast::DeclStmt>(parseVarDecl());
}

ast::Block::Ptr Parser::parseBlock()
{
#ifdef lilang_trace
    trace("Block");
#endif
    expect(CodeType::kLeftBrace);
    auto list = parseStmtList();
    expect(CodeType::kRightBrace);
    return std::make_shared<ast::Block>(list);
}

//********************************************************************
// declaration related
//********************************************************************
ast::Decl::Ptr Parser::parseVarDecl()
{
    expect(CodeType::kLet);
    ast::Obj::List var_list;
    while (true)
    {
        var_list.push_back(std::make_shared<ast::Obj>(nullptr, cur_tok.value));
        if (cur_tok.type != CodeType::kComma)
        {
            break;
        }
        nextToken();
    }
    if (cur_tok.type == CodeType::kAssign)
    {
        nextToken();
        auto rhs = parseExprList();
        expect(CodeType::kSemiColon);
        return std::make_shared<ast::VarDecl>(var_list, rhs);
    }
    else
    {
        auto t = parseType();
        expect(CodeType::kSemiColon);
        return std::make_shared<ast::VarDecl>(var_list, t);
    }
}