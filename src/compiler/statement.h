#ifndef LILANG_COMPILER_STATEMENT
#define LILANG_COMPILER_STATEMENT

#include <memory>
#include <vector>
#include "./ast.h"
#include "./expression.h"

namespace lilang
{
    namespace ast
    {
        class Stmt;

        class IfStmt : public Stmt
        {
        public:
            TokenPos if_pos;
            ExprType condition;
            StmtType if_block;
            StmtType else_block;
            IfStmt() = default;
            IfStmt(TokenPos if_pos, ExprType cond, StmtType if_block, StmtType else_block)
                : if_pos(if_pos), condition(cond), if_block(if_block), else_block(else_block) {}
            inline TokenPos Start()
            {
                return if_pos;
            }
        };

        class WhileStmt : public Stmt
        {
        public:
            TokenPos while_pos;
            ExprType condition;
            StmtType block;
            WhileStmt() = default;
            WhileStmt(TokenPos while_pos, ExprType cond, StmtType block)
                : while_pos(while_pos), condition(cond), block(block) {}
            inline TokenPos Start()
            {
                return while_pos;
            }
        };

        class ForStmt : public Stmt
        {
        public:
            TokenPos for_pos;
            StmtType init;
            ExprType condition;
            StmtType post;
            ForStmt() = default;
            ForStmt(TokenPos for_pos, StmtType init, ExprType cond, StmtType post)
                : for_pos(for_pos), init(init), condition(cond), post(post) {}
            inline TokenPos Start()
            {
                return for_pos;
            }
        };

        class Block : public Stmt
        {
        public:
            TokenPos left_bracket;
            StmtListType stmts;
            Block() = default;
            Block(TokenPos lb, StmtListType stmts) : left_bracket(lb), stmts(stmts) {}
            inline TokenPos Start()
            {
                return left_bracket;
            }
        };
    }
}

#endif