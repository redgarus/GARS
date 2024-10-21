#pragma once

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

using llvm::Function, llvm::Value;

struct Stmt;

struct WarStmt;
struct TrenStmt;
struct ParenStmts;
struct RetStmt;
struct IfStmt;
struct AliveStmt;
struct HighExpr;

struct Expr;
struct AssignExpr;
struct BoolExpr;
struct AddExpr;
struct TermExpr;
struct IDExpr;
struct CallExpr;
struct IntExpr;
struct ArrayExpr;
struct ParenExpr;
struct IndexExpr;

struct Input;

struct ASTVisitor {
    virtual Value *visit(WarStmt&) = 0;
    virtual Function *visit(TrenStmt&) = 0;
    virtual Value *visit(RetStmt&) = 0;
    virtual Value *visit(IfStmt&) = 0;
    virtual Value *visit(AliveStmt&) = 0;
    virtual Value *visit(HighExpr&) = 0;
    virtual Value *visit(ParenStmts&) = 0;

    virtual Value *visit(AssignExpr&) = 0;
    virtual Value *visit(BoolExpr&) = 0;
    virtual Value *visit(AddExpr&) = 0;
    virtual Value *visit(TermExpr&) = 0;
    virtual Value *visit(IDExpr&) = 0;
    virtual Value *visit(CallExpr&) = 0;
    virtual Value *visit(IntExpr&) = 0;
    virtual Value *visit(ArrayExpr&) = 0;
    virtual Value *visit(ParenExpr&) = 0;
    virtual Value *visit(IndexExpr&) = 0;

    virtual Value *visit(Input&) = 0;

    virtual ~ASTVisitor() = default;
};
