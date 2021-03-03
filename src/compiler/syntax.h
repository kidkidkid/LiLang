#ifndef LILANG_COMPILER_SYNTAX
#define LILANG_COMPILER_SYNTAX

#include <map>
#include "./lexical.h"
#include "./ast.h"
#include "./expression.h"
#include "./declaration.h"
#include "./statement.h"

namespace lilang
{
    namespace compiler
    {
        class Parser
        {
        public:
            Parser() = default;
            Parser(CodeToken::List);
            std::shared_ptr<ast::File> parse();
            void printErrors();

            typedef struct Error
            {
                ast::TokenPos pos;
                string_t msg;
            } Error;

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

        private:
            using TokenMap = std::map<CodeType, bool>;
            static TokenMap expression_follow;
            static TokenMap statement_follow;

            // current token
            ast::TokenPos cur_pos;
            CodeToken::List::iterator cur_iter;
            CodeToken::List tokens;
            CodeToken cur_tok;

            //errors
            std::vector<Error> error_list;

            // helper
            void nextToken();
            void exhaust(TokenMap &);
            ast::TokenPos expect(CodeType);
            void expectError(const string_t &msg);

            // expression related
            ast::ExprType parseExpression();
            ast::ExprListType parseExprList();
            ast::ExprType parseBinaryExpression(int);
            ast::ExprType parseUnaryExpression();
            ast::ExprType parsePrimaryExpression();
            ast::ExprType parseIndex(ast::ExprType);
            ast::ExprType parseCallOrConversion(ast::ExprType);
            ast::ExprType parseOperand();
            ast::ExprType parseIdent();
            ast::ExprType parseBasicLit();
            ast::ExprType parseType();
            ast::ExprType tryParseType();
            ast::ExprType parseTypeName();
            ast::ExprType parsePointerType();
            ast::ExprType parseArrayType();
            ast::ExprType parseFuncType();
            ast::FieldType parseField();
            ast::FieldListType parseFnParamters();
            ast::FieldListType parseFnResults();
            ast::ExprType parseFuncLitOrType();

            // statement related
            ast::StmtType parseStmt();
            ast::StmtListType parseStmtList();
            ast::StmtType parseSimpleStmt();
            ast::StmtType parseIfStmt();
            ast::StmtType parseWhileStmt();
            ast::StmtType parseForStmt();
            ast::StmtType parseBlock();
            ast::StmtType parseDeclStmt();
            ast::StmtType parseReturnStmt();

            //declaration related
        };
    }
}

#endif