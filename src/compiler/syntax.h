#ifndef LILANG_COMPILER_SYNTAX
#define LILANG_COMPILER_SYNTAX

#include <map>
#include "./lexical.h"
#include "./ast.h"

namespace lilang
{
    namespace compiler
    {
        class Parser
        {
        public:
            Parser() = default;
            ast::File::Ptr ParseFile(const string_t &);
            ast::File::Ptr ParseTokens(CodeToken::List &);
            ast::File::Ptr ParseString(const string_t &);
            void PrintErrors();

        private:
            using TokenMap = std::map<CodeType, bool>;
            static TokenMap expression_follow;
            static TokenMap statement_follow;
            static TokenMap declaration_start;

            // current token
            ast::TokenPos cur_pos;
            CodeToken::List::iterator cur_iter;
            CodeToken::List tokens;
            CodeToken cur_tok;

            // helper
            void NextToken();
            void Exhaust(TokenMap &);
            ast::TokenPos Expect(CodeType);
            void ExpectError(const string_t &msg);

            // parse the top level
            ast::File::Ptr Parse();

            // expression related
            ast::Expr::Ptr ParseExpression();
            ast::Expr::List ParseExprList();
            ast::Expr::Ptr ParseBinaryExpression(int);
            ast::Expr::Ptr ParseUnaryExpression();
            ast::Expr::Ptr ParsePrimaryExpression();
            ast::Expr::Ptr ParseIndex(ast::Expr::Ptr);
            ast::Expr::Ptr ParseCall(ast::Expr::Ptr);
            ast::Expr::Ptr ParseOperand();
            ast::Expr::Ptr ParseIdent();
            ast::Expr::Ptr ParseBasicLit();
            ast::Expr::Ptr ParseFuncLit();
            // type related
            ast::Expr::Ptr ParseType();
            ast::Expr::Ptr TryParseType();
            ast::Expr::Ptr ParseTypeName();
            ast::Expr::Ptr ParsePointerType();
            ast::Expr::Ptr ParseArrayType();
            ast::FuncType::Ptr ParseFuncType();
            ast::Field::Ptr ParseField();
            ast::Field::List ParseFnParamters();
            ast::Expr::List ParseFnResults();

            // statement related
            ast::Stmt::Ptr ParseStmt();
            ast::Stmt::List ParseStmtList();
            ast::Stmt::Ptr ParseSimpleStmt();
            ast::Stmt::Ptr ParseIfStmt();
            ast::Stmt::Ptr ParseWhileStmt();
            ast::Stmt::Ptr ParseForStmt();
            ast::Block::Ptr ParseBlock();
            ast::Stmt::Ptr ParseVarDeclStmt();
            ast::Stmt::Ptr ParseReturnStmt();

            // declaration related
            ast::Decl::Ptr ParseVarDecl();
            ast::Decl::Ptr ParseFuncDecl();

            // debug trace
            // use constructor and deconstructor to realize facility like defer in Golang
            class Trace
            {
            public:
                static int trace_ident;
                Parser *p;
                Trace() = default;
                Trace(const string_t &msg, Parser *);
                ~Trace();
            };

            // errors
            typedef struct Error
            {
                ast::TokenPos pos;
                string_t msg;
            } Error;
            std::vector<Error> error_list;
        };
    }
}

#endif