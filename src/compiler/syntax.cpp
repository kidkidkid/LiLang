#include <sstream>
#include <iostream>
#include <iomanip>
#include "./syntax.h"

#define RepeatStringLit(N, S)   \
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
Parser::TokenMap Parser::declaration_start = {
    {CodeType::kLet, true},
    {CodeType::kFn, true},
};

//********************************************************************
// helper
//********************************************************************

void Parser::NextToken()
{
#ifdef lilang_trace
    auto tok = tokens[cur_pos];
    std::cout << std::setw(3) << tok.row_number << ":" << std::setw(3) << tok.column_number << ":";
    RepeatStringLit(Trace::trace_ident * 2, ".");
    std::cout << "\"" << cur_tok.value << "\"" << std::endl;
#endif
    if (cur_iter != tokens.end() && cur_tok.type != CodeType::kEOF)
    {
        cur_iter++;
        cur_pos++;
        cur_tok = *cur_iter;
        while (cur_tok.type == CodeType::kComment) // skip comment token
        {
            cur_iter++;
            cur_pos++;
            cur_tok = *cur_iter;
        }
    }
}

ast::TokenPos Parser::Expect(CodeType t)
{
    if (cur_tok.type != t)
    {
        stringstream_t ss;
        ss << CodeToken::Type2Str(t) << " expected, found " << cur_tok.value;
        error_list.push_back({cur_pos, ss.str()});
    }
    NextToken();
    return cur_pos;
}

void Parser::ExpectError(const string_t &msg)
{
    stringstream_t ss;
    ss << msg << " expected, found " << cur_tok.value;
    error_list.push_back({cur_pos, ss.str()});
}

void Parser::Exhaust(TokenMap &mp)
{
    while (cur_tok.type != CodeType::kEOF && mp[cur_tok.type] == false)
    {
        NextToken();
    }
}

int Parser::Trace::trace_ident = 0;
Parser::Trace::Trace(const string_t &msg, Parser *p) : p(p)
{
    auto tok = p->tokens[p->cur_pos];
    std::cout << std::setw(3) << tok.row_number << ":" << std::setw(3) << tok.column_number << ":";
    RepeatStringLit(trace_ident * 2, ".");
    RepeatStringLit(1, msg);
    RepeatStringLit(1, "(\n");
    trace_ident++;
}

Parser::Trace::~Trace()
{
    trace_ident--;
    auto tok = p->tokens[p->cur_pos];
    std::cout << std::setw(3) << tok.row_number << ":" << std::setw(3) << tok.column_number << ":";
    RepeatStringLit(trace_ident * 2, ".");
    RepeatStringLit(1, ")\n");
}

void Parser::PrintErrors()
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
// file related
//********************************************************************

ast::File::Ptr Parser::ParseFile(const string_t &file_name)
{
    CodeError::List err_list;
    auto tok_list = LexicalParser::ParseFile(file_name, err_list);
    this->tokens = tok_list;
    this->cur_pos = 0;
    this->cur_iter = tokens.begin();
    this->cur_tok = *cur_iter;
    return Parse();
}

ast::File::Ptr Parser::ParseString(const string_t &str)
{
    CodeError::List err_list;
    auto tok_list = LexicalParser::ParseString(str, err_list);
    this->tokens = tok_list;
    this->cur_pos = 0;
    this->cur_iter = tokens.begin();
    this->cur_tok = *cur_iter;
    return Parse();
}

ast::File::Ptr Parser::ParseTokens(CodeToken::List &list)
{
    this->tokens = list;
    this->cur_pos = 0;
    this->cur_iter = tokens.begin();
    this->cur_tok = *cur_iter;
    return Parse();
}

ast::File::Ptr Parser::Parse()
{
    ast::File::Ptr file = std::make_shared<ast::File>();
    while (true)
    {
        switch (cur_tok.type)
        {
        case CodeType::kLet:
            file->AddDecl(ParseVarDecl());
            break;
        case CodeType::kFn:
            file->AddDecl(ParseFuncDecl());
            break;
        case CodeType::kEOF:
            return file;
        default:
        {
            ExpectError("declaration");
            Exhaust(declaration_start);
        }
        break;
        }
    }
    return file;
}

//********************************************************************
// expression related
//********************************************************************

ast::Expr::Ptr Parser::ParseExpression()
{
#ifdef lilang_trace
    trace("Expression");
#endif
    return ParseBinaryExpression(1);
}

// expression{, expression}
ast::Expr::List Parser::ParseExprList()
{
#ifdef lilang_trace
    trace("ExpressionList");
#endif
    ast::Expr::List list;
    while (true)
    {
        list.push_back(ParseExpression()); // rvalue
        if (cur_tok.type != CodeType::kComma)
        {
            break;
        }
        Expect(CodeType::kComma);
    }
    return list;
}

// The power of Recursive!
ast::Expr::Ptr Parser::ParseBinaryExpression(int before_prec)
{
#ifdef lilang_trace
    trace("BinaryExpr");
#endif
    auto x = ParseUnaryExpression();
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
        Expect(op);
        auto right = ParseBinaryExpression(cur_prec + 1);
        x = std::make_shared<ast::BinaryExpr>(x, op, right);
    }
}

ast::Expr::Ptr Parser::ParseUnaryExpression()
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
        NextToken();
        auto ue = ParseUnaryExpression();
        return std::make_shared<ast::UnaryExpr>(t, ue);
    }
    default:
        return ParsePrimaryExpression();
    }
}

// type(expression)
// operand[expression]
// operand(arg1, arg2, arg3)
ast::Expr::Ptr Parser::ParsePrimaryExpression()
{
#ifdef lilang_trace
    trace("PrimaryExpr");
#endif
    auto p = ParseOperand();
    while (true)
    {
        if (cur_tok.type == CodeType::kLeftParenthese)
        {
            p = ParseCall(p);
        }
        else if (cur_tok.type == CodeType::kLeftBracket)
        {
            p = ParseIndex(p);
        }
        else
        {
            break;
        }
    }
    return p;
}

ast::Expr::Ptr Parser::ParseOperand()
{
#ifdef lilang_trace
    trace("Operand");
#endif
    switch (cur_tok.type)
    {
    case CodeType::kIdentifier: // variable
    {
        return ParseIdent();
    }
    case CodeType::kLeftParenthese: // (expression)
    {
        Expect(CodeType::kLeftParenthese);
        auto e = ParseExpression();
        Expect(CodeType::kRightParenthese);
        return std::make_shared<ast::ParenExpr>(e);
    }
    case CodeType::kStringLiteral:
    case CodeType::kNumber:
    case CodeType::kFloat:
        return ParseBasicLit();
    case CodeType::kFn:
    {
        // function literal OR function type
        auto t = ParseFuncType();
        if (cur_tok.type == CodeType::kLeftBrace)
        {
            return std::make_shared<ast::FuncLit>(t, ParseBlock());
        }
        else
        {
            return t;
        }
    }
    default:
        break;
    }
    // type cast
    auto t = TryParseType();
    if (t != nullptr)
    {
        return t;
    }
    ExpectError("operand");
    Exhaust(expression_follow);
    return std::make_shared<ast::BadExpr>();
}

ast::Expr::Ptr Parser::ParseIdent()
{
    auto ident = std::make_shared<ast::Ident>(cur_tok.value);
    NextToken();
    return ident;
}

ast::Expr::Ptr Parser::ParseCall(ast::Expr::Ptr e)
{
#ifdef lilang_trace
    trace("Call");
#endif
    ast::Expr::List arg_list;
    Expect(CodeType::kLeftParenthese);
    while (true)
    {
        arg_list.push_back(ParseExpression());
        if (cur_tok.type == CodeType::kComma)
        {
            NextToken();
            continue;
        }
        else
        {
            break;
        }
    }
    Expect(CodeType::kRightParenthese);
    return std::make_shared<ast::CallExpr>(e, arg_list);
}

ast::Expr::Ptr Parser::ParseIndex(ast::Expr::Ptr o)
{
#ifdef lilang_trace
    trace("Index");
#endif
    Expect(CodeType::kLeftBracket);
    auto i = ParseExpression();
    Expect(CodeType::kRightBracket);
    return std::make_shared<ast::IndexExpr>(o, i);
}

// parse type
ast::Expr::Ptr Parser::ParseType()
{
    auto t = TryParseType();
    if (t == nullptr)
    {
        ExpectError("type");
        NextToken();
        return std::make_shared<ast::BadExpr>();
    }
    return t;
}

// return type or null if no type found
ast::Expr::Ptr Parser::TryParseType()
{
    switch (cur_tok.type)
    {
    case CodeType::kMultiply:
        return ParsePointerType();
    case CodeType::kLeftBracket:
        return ParseArrayType();
    case CodeType::kIdentifier:
        return ParseTypeName();
    case CodeType::kFn:
        return ParseFuncType();
    default:
        return nullptr;
    }
}

// identifier typename
ast::Expr::Ptr Parser::ParseTypeName()
{
#ifdef lilang_trace
    trace("TypeName");
#endif
    auto t = std::make_shared<ast::Ident>(cur_tok.value);
    NextToken();
    return t;
}

// ****int
ast::Expr::Ptr Parser::ParsePointerType()
{
#ifdef lilang_trace
    trace("PointerExpr");
#endif
    Expect(CodeType::kMultiply);
    auto t = ParseType();
    return std::make_shared<ast::StarExpr>(t);
}

// [][][]*int
ast::Expr::Ptr Parser::ParseArrayType()
{
#ifdef lilang_trace
    trace("ArrayExpr");
#endif
    Expect(CodeType::kLeftBracket);
    Expect(CodeType::kRightBracket);
    return std::make_shared<ast::ArrayType>(ParseType());
}

// fn(int, int)()
ast::FuncType::Ptr Parser::ParseFuncType()
{
#ifdef lilang_trace
    trace("FuncType");
#endif
    Expect(CodeType::kFn);
    auto args = ParseFnParamters();
    auto rets = ParseFnResults();
    return std::make_shared<ast::FuncType>(args, rets);
}

ast::Expr::Ptr Parser::ParseFuncLit()
{
#ifdef lilang_trace
    trace("FuncLit");
#endif
    Expect(CodeType::kFn);
    auto type = ParseFuncType();
    auto body = ParseBlock();
    return std::make_shared<ast::FuncLit>(type, body);
}

// (int x , int y, float z)
// (int, float)
ast::Field::List Parser::ParseFnParamters()
{
#ifdef lilang_trace
    trace("Parametes");
#endif
    ast::Field::List params;
    Expect(CodeType::kLeftParenthese);
    if (cur_tok.type != CodeType::kRightParenthese)
    {
        while (true)
        {
            params.push_back(ParseField());
            if (cur_tok.type == CodeType::kComma)
            {
                NextToken();
                continue;
            }
            else
            {
                break;
            }
        }
    }
    Expect(CodeType::kRightParenthese);
    return params;
}

// int
// ()
// (int, int)
ast::Expr::List Parser::ParseFnResults()
{
#ifdef lilang_trace
    trace("Results");
#endif
    ast::Expr::List returns;
    // have multiple retuen values
    if (cur_tok.type == CodeType::kLeftParenthese)
    {
        NextToken();
        if (cur_tok.type != CodeType::kRightParenthese)
        {
            while (true)
            {
                returns.push_back(ParseType());
                if (cur_tok.type == CodeType::kComma)
                {
                    NextToken();
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        Expect(CodeType::kRightParenthese);
    }
    else // have exactly one type or no return values
    {
        auto t = TryParseType();
        if (t != nullptr)
        {
            returns.push_back(t);
        }
    }
    return returns;
}

// type identifier OR type
ast::Field::Ptr Parser::ParseField()
{
    auto t = ParseType();
    if (cur_tok.type == CodeType::kComma ||
        cur_tok.type == CodeType::kRightParenthese ||
        cur_tok.type == CodeType::kEOF)
    {
        return std::make_shared<ast::Field>("_", t);
    }
    auto name = cur_tok.value;
    NextToken();
    return std::make_shared<ast::Field>(name, t);
}

// literal
ast::Expr::Ptr Parser::ParseBasicLit()
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
    NextToken();
    return lit;
}

//********************************************************************
// statement related
//********************************************************************

// semicolon is expect in each statement, some statements dont end with semicolon
ast::Stmt::Ptr Parser::ParseStmt()
{
#ifdef lilang_trace
    trace("Statement");
#endif
    switch (cur_tok.type)
    {
    case CodeType::kIf:
        return ParseIfStmt();
    case CodeType::kWhile:
        return ParseWhileStmt();
    case CodeType::kFor:
        return ParseForStmt();
    case CodeType::kLeftBrace:
        return ParseBlock();
    case CodeType::kReturn:
        return ParseReturnStmt();
    case CodeType::kLet:
    {
        auto e = ParseVarDeclStmt();
        Expect(CodeType::kSemiColon);
        return e;
    }
    case CodeType::kSemiColon:
    {
        Expect(CodeType::kSemiColon);
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
    {
        auto s = ParseSimpleStmt();
        Expect(CodeType::kSemiColon);
        return s;
    }
    default:
        break;
    };
    ExpectError("statement");
    Exhaust(statement_follow); // if can not parse stmt, exhaust until ; OR }
    return std::make_shared<ast::EmptyStmt>();
}

ast::Stmt::List Parser::ParseStmtList()
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
        list.push_back(ParseStmt()); // rvalue
    }
    return list;
}

// assign
ast::Stmt::Ptr Parser::ParseSimpleStmt()
{
#ifdef lilang_trace
    trace("SimpleStatement");
#endif
    auto lhs = ParseExprList();
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
    case CodeType::kShortAssign:
    {
        NextToken();
        auto rhs = ParseExprList();
        return std::make_shared<ast::AssignStmt>(lhs, rhs);
    }
    default:
        break;
    }
    if (lhs.size() > 1)
    {
        ExpectError("one expression");
        return std::make_shared<ast::EmptyStmt>();
    }
    // todo, maybe add ++/-- or other features
    return std::make_shared<ast::ExprStmt>(lhs[0]); // expression statement
}

ast::Stmt::Ptr Parser::ParseIfStmt()
{
#ifdef lilang_trace
    trace("IfStatement");
#endif
    Expect(CodeType::kIf);
    Expect(CodeType::kLeftParenthese);
    auto cond = ParseExpression();
    Expect(CodeType::kRightParenthese);
    auto if_block = ParseBlock();
    ast::Stmt::Ptr else_block = nullptr;
    if (cur_tok.type == CodeType::kElse)
    {
        NextToken();
        if (cur_tok.type == CodeType::kIf)
        {
            else_block = ParseIfStmt();
        }
        else
        {
            else_block = ParseBlock();
        }
    }
    return std::make_shared<ast::IfStmt>(cond, if_block, else_block);
}

ast::Stmt::Ptr Parser::ParseWhileStmt()
{
#ifdef lilang_trace
    trace("WhileStatement");
#endif
    Expect(CodeType::kWhile);
    Expect(CodeType::kLeftParenthese);
    auto e = ParseExpression();
    Expect(CodeType::kRightParenthese);
    auto b = ParseBlock();
    return std::make_shared<ast::WhileStmt>(e, b);
}

ast::Stmt::Ptr Parser::ParseForStmt()
{
#ifdef lilang_trace
    trace("IfStatement");
#endif
    Expect(CodeType::kFor);
    Expect(CodeType::kLeftParenthese);
    // init statement
    ast::Stmt::Ptr init;
    if (cur_tok.type == CodeType::kLet)
    {
        init = ParseVarDeclStmt();
    }
    else
    {
        init = ParseSimpleStmt();
        Expect(CodeType::kSemiColon);
    }
    auto cond = ParseExpression();
    Expect(CodeType::kSemiColon);
    auto post = ParseSimpleStmt();
    Expect(CodeType::kRightParenthese);
    auto b = ParseBlock();
    return std::make_shared<ast::ForStmt>(init, cond, post, b);
}

ast::Stmt::Ptr Parser::ParseReturnStmt()
{
#ifdef lilang_trace
    trace("ReturnStatement");
#endif
    Expect(CodeType::kReturn);
    auto rhs = ParseExprList();
    Expect(CodeType::kSemiColon);
    return std::make_shared<ast::RetStmt>(rhs);
}

// let x, y, z type;
// let x, y, z = e1, e2, e3;
ast::Stmt::Ptr Parser::ParseVarDeclStmt()
{
    return std::make_shared<ast::DeclStmt>(ParseVarDecl());
}

ast::Block::Ptr Parser::ParseBlock()
{
#ifdef lilang_trace
    trace("Block");
#endif
    Expect(CodeType::kLeftBrace);
    auto list = ParseStmtList();
    Expect(CodeType::kRightBrace);
    return std::make_shared<ast::Block>(list);
}

//********************************************************************
// declaration related
//********************************************************************
ast::Decl::Ptr Parser::ParseVarDecl()
{
#ifdef lilang_trace
    trace("VarDecl");
#endif
    Expect(CodeType::kLet);
    std::vector<string_t> names;
    while (true)
    {
        names.push_back(cur_tok.value);
        NextToken(); // skip identifier
        if (cur_tok.type != CodeType::kComma)
        {
            break;
        }
        NextToken(); // skip comma
    }
    if (cur_tok.type == CodeType::kAssign)
    {
        NextToken();
        auto rhs = ParseExprList();
        Expect(CodeType::kSemiColon);
        return std::make_shared<ast::VarDecl>(names, rhs);
    }
    else
    {
        auto t = ParseType();
        Expect(CodeType::kSemiColon);
        return std::make_shared<ast::VarDecl>(names, t);
    }
}

ast::Decl::Ptr Parser::ParseFuncDecl()
{
#ifdef lilang_trace
    trace("FuncDecl");
#endif
    Expect(CodeType::kFn);
    auto name = cur_tok.value;
    NextToken();
    auto args = ParseFnParamters();
    auto rets = ParseFnResults();
    auto type = std::make_shared<ast::FuncType>(args, rets);
    auto body = ParseBlock();
    return std::make_shared<ast::FuncDecl>(name, type, body);
}