#include <sstream>
#include <iostream>
#include <iomanip>
#include "./syntax.h"
#include "./expression.h"
#include "./declaration.h"

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

ast::ExprType Parser::parseExpression()
{
#ifdef lilang_trace
    trace("Expression");
#endif
    return parseBinaryExpression(1);
}

// expression{, expression}
ast::ExprListType Parser::parseExprList()
{
#ifdef lilang_trace
    trace("ExpressionList");
#endif
    ast::ExprListType list;
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
ast::ExprType Parser::parseBinaryExpression(int before_prec)
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

ast::ExprType Parser::parseUnaryExpression()
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
        auto pos = cur_pos;
        auto t = cur_tok.type;
        nextToken();
        auto ue = parseUnaryExpression();
        return std::make_shared<ast::UnaryExpr>(pos, t, ue);
    }
    default:
        return parsePrimaryExpression();
    }
}

// type(expression)
// operand[expression]
// operand(arg1, arg2, arg3)
ast::ExprType Parser::parsePrimaryExpression()
{
#ifdef lilang_trace
    trace("PrimaryExpr");
#endif
    auto p = parseOperand();
    while (true)
    {
        if (cur_tok.type == CodeType::kLeftParenthese)
        {
            p = parseCallOrConversion(p);
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

ast::ExprType Parser::parseCallOrConversion(ast::ExprType e)
{
#ifdef lilang_trace
    trace("CallOrConversion");
#endif
    ast::ExprListType arg_list;
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

ast::ExprType Parser::parseIndex(ast::ExprType o)
{
#ifdef lilang_trace
    trace("Index");
#endif
    expect(CodeType::kLeftBracket);
    auto i = parseExpression();
    expect(CodeType::kRightBracket);
    return std::make_shared<ast::IndexExpr>(o, i);
}

ast::ExprType Parser::parseOperand()
{
#ifdef lilang_trace
    trace("Operand");
#endif
    ast::ExprType e;
    switch (cur_tok.type)
    {
    case CodeType::kIdentifier:
        e = parseIdent();
        return e;
    case CodeType::kLeftParenthese: // (expression) or (type)
    {
        auto l = cur_pos;
        nextToken();
        e = parseExpression();
        auto r = expect(CodeType::kRightParenthese);
        e = std::make_shared<ast::ParenExpr>(l, e, r);
        return e;
    }
    case CodeType::kStringLiteral:
    case CodeType::kNumber:
    case CodeType::kFloat:
        return parseBasicLit();
    case CodeType::kFn:
        return parseFuncLitOrType();
    default:
        break;
    }

    // try parse type, operand may be conversion: operand -> type(expression)
    // (type)identifier is not allowed
    auto t = tryParseType();
    if (t == nullptr)
    {
        auto l = cur_pos;
        expectError("operand");
        exhaust(expression_follow); // necessary?
        return std::make_shared<ast::BadExpr>(l, cur_pos);
    }
    return t;
}

// parse type
ast::ExprType Parser::parseType()
{
    auto t = tryParseType();
    if (t == nullptr)
    {
        expectError("type");
        return std::make_shared<ast::BadExpr>(cur_pos, cur_pos);
    }
    return t;
}

// return type or null if no type found
ast::ExprType Parser::tryParseType()
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
ast::ExprType Parser::parseTypeName()
{
#ifdef lilang_trace
    trace("TypeName");
#endif
    return parseIdent();
}

// ****int
ast::ExprType Parser::parsePointerType()
{
#ifdef lilang_trace
    trace("StarExpr");
#endif
    ast::TokenPos p = cur_pos;
    expect(CodeType::kMultiply);
    auto e = parseType();
    return std::make_shared<ast::StarExpr>(p, e);
}

// [][][]*int
ast::ExprType Parser::parseArrayType()
{
#ifdef lilang_trace
    trace("ArrayExpr");
#endif
    ast::TokenPos p = cur_pos;
    expect(CodeType::kLeftBracket);
    expect(CodeType::kRightBracket);
    auto t = parseType();
    return std::make_shared<ast::ArrayType>(p, t);
}

// fn()(){}
ast::ExprType Parser::parseFuncLitOrType()
{
    auto f = parseFuncType();
    if (cur_tok.type != CodeType::kLeftBrace)
    {
        return f;
    }
    else
    {
        auto stmt = parseBlock();
        return std::make_shared<ast::FuncLit>(f, stmt);
    }
}

// fn(int, int)()
ast::ExprType Parser::parseFuncType()
{
#ifdef lilang_trace
    trace("FuncType");
#endif
    ast::TokenPos fn_pos = cur_pos;
    nextToken(); //skip fn keyword
    auto params = parseFnParamters();
    auto returns = parseFnResults();
    return std::make_shared<ast::FuncType>(fn_pos, params, returns);
}

// (int x , int y, float z)
// (int, float)
ast::FieldListType Parser::parseFnParamters()
{
#ifdef lilang_trace
    trace("Parametes");
#endif
    ast::FieldListType params;
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
ast::FieldListType Parser::parseFnResults()
{
#ifdef lilang_trace
    trace("Results");
#endif
    ast::FieldListType returns;
    // have multiple retuen values
    if (cur_tok.type == CodeType::kLeftParenthese)
    {
        nextToken();
        if (cur_tok.type != CodeType::kRightParenthese)
        {
            while (true)
            {
                returns.push_back(std::make_shared<ast::Field>(parseType()));
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
            returns.push_back(std::make_shared<ast::Field>(t));
        }
    }
    return returns;
}

// type identifier OR type
ast::FieldType Parser::parseField()
{
    auto t = parseType();
    if (cur_tok.type == CodeType::kComma ||
        cur_tok.type == CodeType::kSemiColon ||
        cur_tok.type == CodeType::kRightParenthese ||
        cur_tok.type == CodeType::kEOF)
    {
        return std::make_shared<ast::Field>(t);
    }
    auto i = parseIdent();
    return std::make_shared<ast::Field>(t, i);
}

// identifier
ast::ExprType Parser::parseIdent()
{
    if (cur_tok.type != CodeType::kIdentifier)
    {
        auto p = cur_pos;
        expectError("identifier");
        nextToken();
        return std::make_shared<ast::BadExpr>(p, p);
    }
    auto ident = std::make_shared<ast::Ident>();
    ident->name = cur_tok.value;
    ident->pos = cur_pos;
    // predicate identifier is type
    if (cur_tok.value == "int" || cur_tok.value == "float" || cur_tok.value == "string")
    {
        ident->kind = ast::IdentType::kType;
    }
    else
    {
        ident->kind = ast::IdentType::kVariable; //default
    }
    nextToken();
    return ident;
}

// identifier {, identifier}
ast::ExprListType Parser::parseIdentList()
{
#ifdef lilang_trace
    trace("IdentList");
#endif
    ast::ExprListType list;
    while (true)
    {
        list.push_back(parseIdent());
        if (cur_tok.type != CodeType::kComma)
        {
            break;
        }
        expect(CodeType::kComma);
    }
    return list;
}

// literal
ast::ExprType Parser::parseBasicLit()
{
    auto lit = std::make_shared<ast::BasicLiteral>();
    lit->kind = cur_tok.type;
    lit->pos = cur_pos;
    lit->value = cur_tok.value;
    nextToken();
    return lit;
}

//********************************************************************
// statement related
//********************************************************************

// semicolon is expect in each statement, some statements dont end with semicolon
ast::StmtType Parser::parseStmt()
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
        auto p = cur_pos;
        nextToken();
        return std::make_shared<ast::EmptyStmt>(p);
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
    auto l = cur_pos;
    expectError("statement");
    exhaust(statement_follow); // if can not parse stmt, exhaust until ; OR }
    return std::make_shared<ast::BadStmt>(l, cur_pos);
}

ast::StmtListType Parser::parseStmtList()
{
#ifdef lilang_trace
    trace("StatementList");
#endif
    ast::StmtListType list;
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
ast::StmtType Parser::parseSimpleStmt()
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
        auto p = cur_pos;
        nextToken();
        auto rhs = parseExprList();
        expect(CodeType::kSemiColon);
        return std::make_shared<ast::AssignStmt>(lhs, p, rhs);
    }
    default:
        break;
    }
    if (lhs.size() > 1)
    {
        expectError("one expression");
        return std::make_shared<ast::BadStmt>(lhs[0]->Start(), cur_pos);
    }
    expect(CodeType::kSemiColon);
    // todo, maybe add ++/-- or other features
    return std::make_shared<ast::ExprStmt>(lhs[0]); // expression statement
}

ast::StmtType Parser::parseIfStmt()
{
#ifdef lilang_trace
    trace("IfStatement");
#endif
    auto p = cur_pos;
    expect(CodeType::kIf);
    expect(CodeType::kLeftParenthese);
    auto cond = parseExpression();
    expect(CodeType::kRightParenthese);
    auto if_block = parseBlock();
    ast::StmtType else_block = nullptr;
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
    return std::make_shared<ast::IfStmt>(p, cond, if_block, else_block);
}

ast::StmtType Parser::parseWhileStmt()
{
#ifdef lilang_trace
    trace("WhileStatement");
#endif
    return nullptr;
}

ast::StmtType Parser::parseForStmt()
{
#ifdef lilang_trace
    trace("IfStatement");
#endif
    return nullptr;
}

ast::StmtType Parser::parseReturnStmt()
{
#ifdef lilang_trace
    trace("ReturnStatement");
#endif
    auto p = cur_pos;
    expect(CodeType::kReturn);
    auto rhs = parseExprList();
    expect(CodeType::kSemiColon);
    return std::make_shared<ast::RetStmt>(p, rhs);
}

// let x, y, z type;
// let x, y, z = e1, e2, e3;
ast::StmtType Parser::parseDeclStmt()
{
#ifdef lilang_trace
    trace("DeclStatement");
#endif
    auto p = cur_pos;
    expect(CodeType::kLet);
    auto lhs = parseIdentList();
    if (cur_tok.type == CodeType::kAssign)
    {
        nextToken();
        auto rhs = parseExprList();
        expect(CodeType::kSemiColon);
        auto decl = std::make_shared<ast::ValDecl>(p, lhs, rhs);
        return std::make_shared<ast::DeclStmt>(decl);
    }
    else
    {
        auto t = parseType();
        expect(CodeType::kSemiColon);
        auto decl = std::make_shared<ast::ValDecl>(p, lhs, t);
        return std::make_shared<ast::DeclStmt>(decl);
    }
}

ast::StmtType Parser::parseBlock()
{
#ifdef lilang_trace
    trace("Block");
#endif
    auto p = cur_pos;
    expect(CodeType::kLeftBrace);
    auto list = parseStmtList();
    expect(CodeType::kRightBrace);
    return std::make_shared<ast::Block>(p, list);
}