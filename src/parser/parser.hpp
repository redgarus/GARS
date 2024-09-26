#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.hpp"

namespace Parser {

using namespace AST;

static TOKEN nextToken();
static shared_ptr<Table> nextTable();

unique_ptr<Input> ParseInput();

static unique_ptr<Stmt> ParseStatement(),
                ParseIfStmt(),
                ParseTrenStmt(),
                ParseRetStmt(),
                ParseAliveStmt(),
                ParseWarStmt(),
                ParseHighExpr(),
                ParseParenStmts();


static unique_ptr<Expr> ParseExpression(),
                ParseBoolExpr(),
                ParseAddExpr(),
                ParseTermExpr(),
                ParseFactor();
                
}
#endif
