#include <sstream>
#include <iostream>
#include "./syntax.h"
#include "./expression.h"

#define repeatStringLit(N, S)   \
    for (int i = 0; i < N; i++) \
    {                           \
        std::cout << S;         \
    }

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
Parser::Trace::Trace(const string_t &msg)
{
    repeatStringLit(trace_ident * 2, " ");
    repeatStringLit(1, msg);
    repeatStringLit(1, "(\n");
    trace_ident++;
}

Parser::Trace::~Trace()
{
    trace_ident--;
    repeatStringLit(trace_ident * 2, " ");
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

// expression related
ast::ExprType Parser::parseIdent()
{
    auto ident = std::make_shared<ast::Ident>();
    ident->name = cur_tok.value;
    ident->pos = cur_pos;
    // judge whether identifier is type
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

ast::ExprType Parser::parseBasicLit()
{
    auto lit = std::make_shared<ast::BasicLiteral>();
    lit->kind = cur_tok.type;
    lit->pos = cur_pos;
    lit->value = cur_tok.value;
    nextToken();
    return lit;
}

ast::ExprType Parser::parseOperand()
{
    ast::ExprType e;
    switch (cur_tok.type)
    {
    case CodeType::kIdentifier:
        e = parseIdent();
        nextToken();
        return e;
    case CodeType::kLeftParenthese:
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
        return parseFuncLit();
    default:
        break;
    }

    // try parse type, operand may be conversion: operand -> type(expression)
    return nullptr;
}

// return type or null if no type found
ast::ExprType Parser::parseType()
{
    switch (cur_tok.type)
    {
    case CodeType::kMultiply:
    {
        ast::TokenPos p = cur_pos;
        nextToken();
        auto e = parseType();
        return std::make_shared<ast::StarExpr>(p, e);
    }
    case CodeType::kLeftBracket:
    {
        ast::TokenPos l = cur_pos;
        nextToken();
        ast::TokenPos r = expect(CodeType::kRightBracket);
        auto e = parseType();
        return std::make_shared<ast::ArrayType>(l, e, r);
    }
    case CodeType::kIdentifier:
        return parseIdent();
    case CodeType::kFn:
        return parseFuncType();
    default:
        return nullptr;
    }
}

// try parse identifier or type, may return null
ast::ExprType Parser::tryIdentOrType()
{
    if (cur_tok.type == CodeType::kIdentifier)
    {
        return parseIdent();
    }
    else
    {
        return parseType();
    }
}

// fn(int, int)()
ast::ExprType Parser::parseFuncType()
{
#ifdef lilang_trace
    Trace t("FuncType");
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
    Trace("Parametes");
#endif
    ast::FieldListType params;
    expect(CodeType::kLeftParenthese);
    if (cur_tok.type != CodeType::kRightParenthese)
    {
        while (true)
        {
            auto field = parseField();
            params.push_back(field);
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
    Trace("Results");
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
                auto field = parseField();
                returns.push_back(field);
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
    else // have exactly one value or no value
    {
        auto t = tryIdentOrType();
        if (t != nullptr)
        {
            auto f = std::make_shared<ast::Field>(t);
            returns.push_back(f);
        }
    }
    return returns;
}

// type identifier OR type
ast::FieldType Parser::parseField()
{
    auto t = tryIdentOrType();
    if (t == nullptr)
    {
        expectError("type");
        t = std::make_shared<ast::BadExpr>(cur_pos, cur_pos);
        nextToken();
    }
    if (cur_tok.type == CodeType::kComma ||
        cur_tok.type == CodeType::kSemiColon ||
        cur_tok.type == CodeType::kRightParenthese ||
        cur_tok.type == CodeType::kEOF)
    {
        return std::make_shared<ast::Field>(t);
    }
    else if (cur_tok.type == CodeType::kIdentifier)
    {
        auto i = parseIdent();
        return std::make_shared<ast::Field>(t, i);
    }
    else
    {
        expectError("identifier");
        auto i = std::make_shared<ast::BadExpr>(cur_pos, cur_pos);
        return std::make_shared<ast::Field>(t, i);
    }
}

ast::ExprType Parser::parseFuncLit()
{
    return nullptr;
}

ast::ExprType Parser::parseExpression()
{
    return nullptr;
}