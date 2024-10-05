#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace AST {
    
class ASTVisitor {
public:
    virtual void visit(WarStmt&) = 0;

};


class Stmt;

class WarStmt;
class TrenStmt;

class ParenStmts;
class RetStmt;

class IfStmt;
class AliveStmt;

class HighExpr;


class Expr;

class AssignExpr;
class BoolExpr;
class AddExpr;
class TermExpr;

class IDExpr;
class CallExpr;
class IntExpr;
class ArrayExpr;


class Input;
    
}
#endif
