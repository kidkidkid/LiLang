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
            ast::Expr::Ptr parseExpression();
            ast::Expr::List parseExprList();
            ast::Expr::Ptr parseBinaryExpression(int);
            ast::Expr::Ptr parseUnaryExpression();
            ast::Expr::Ptr parsePrimaryExpression();
            ast::Expr::Ptr parseIndex(ast::Expr::Ptr);
            ast::Expr::Ptr parseCall(ast::Expr::Ptr);
            ast::Expr::Ptr parseCast(ast::Type::Ptr);
            ast::Expr::Ptr parseOperand();
            ast::Expr::Ptr tryParseOperandName();
            ast::Expr::Ptr parseBasicLit();
            //type related
            ast::Type::Ptr parseType();
            ast::Type::Ptr tryParseType();
            ast::Type::Ptr parseTypeName();
            ast::Type::Ptr parsePointerType();
            ast::Type::Ptr parseArrayType();
            ast::Type::Ptr parseFuncType();
            ast::Obj::Ptr parseField();
            ast::Obj::List parseFnParamters();
            ast::Type::List parseFnResults();

            // statement related
            ast::Stmt::Ptr parseStmt();
            ast::Stmt::List parseStmtList();
            ast::Stmt::Ptr parseSimpleStmt();
            ast::Stmt::Ptr parseIfStmt();
            ast::Stmt::Ptr parseWhileStmt();
            ast::Stmt::Ptr parseForStmt();
            ast::Stmt::Ptr parseBlock();
            ast::Stmt::Ptr parseVarDeclStmt();
            ast::Stmt::Ptr parseReturnStmt();

            //declaration related
            ast::Decl::Ptr parseVarDecl();
            ast::Decl::Ptr parseFuncDecl();
        };
    }
}

#endif