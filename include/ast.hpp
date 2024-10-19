#pragma once

#include <memory>
#include <string>
#include <vector>

#include "visitor.hpp"
#include "ast_visitor.hpp"
#include "type.hpp"
#include "token.hpp"
#include "codegen.hpp"

using std::shared_ptr, std::unique_ptr, std::make_unique, std::make_shared, std::string, std::vector, std::pair;

// Abstract Classes

struct Node {
    virtual Value *accept(ASTVisitor&) = 0;

    virtual ~Node() = default;
};

struct Stmt: public Node {
    virtual ~Stmt() = default;
};

struct Expr: public Node {
    virtual shared_ptr<ValueType> getType() const = 0;
    virtual ~Expr() = default;
};

// Expressions

struct AssignExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    shared_ptr<ValueType> type;

    shared_ptr<ValueType> getType() const { return type; }
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    AssignExpr(unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> type)
        : LHS(std::move(lhs)), RHS(std::move(rhs)), type(type) {}
};


struct BoolExpr: public Expr {    
    unique_ptr<Expr> LHS, RHS;
    TOKEN::lexeme OP;
    shared_ptr<ValueType> type;

    shared_ptr<ValueType> getType() const { return type; }
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }
    
    BoolExpr(TOKEN::lexeme OP, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> type)
        : OP(OP), LHS(std::move(lhs)), RHS(std::move(rhs)), type(type) {}
};


struct AddExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    TOKEN::lexeme OP;
    shared_ptr<ValueType> type;

    shared_ptr<ValueType> getType() const { return type; }

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }
    
    AddExpr(TOKEN::lexeme OP, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> type)
        : OP(OP), LHS(std::move(lhs)), RHS(std::move(rhs)), type(type) {}
};

struct TermExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    TOKEN::lexeme OP;
    shared_ptr<ValueType> type;

    shared_ptr<ValueType> getType() const { return type; }
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }
    
    TermExpr(TOKEN::lexeme OP, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> type)
        : OP(OP), LHS(std::move(lhs)), RHS(std::move(rhs)), type(type) {}
};


struct IDExpr: public Expr {
    string name;
    shared_ptr<ValueType> type;

    shared_ptr<ValueType> getType() const { return type; }

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }
    
    IDExpr(const string& name, shared_ptr<ValueType> type)
        : name(name), type(type) {}
};


struct CallExpr: public Expr {
    vector<unique_ptr<Expr>> args;
    shared_ptr<ValueType> type;
    string name;

    shared_ptr<ValueType> getType() const { return type; }

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    CallExpr(const string& name, vector<unique_ptr<Expr>> args, shared_ptr<ValueType> type)
        : name(name), args(std::move(args)), type(type) {}
};

struct IntExpr: public Expr {
    shared_ptr<ValueType> type;
    ll value;

    shared_ptr<ValueType> getType() const { return type; }
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    IntExpr(ll val) : value(val), type(make_shared<IntType>()) {}
};

struct ArrayExpr: public Expr {
    vector<unique_ptr<Expr>> elements;
    shared_ptr<ValueType> type;

    shared_ptr<ValueType> getType() const { return type; }
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    ArrayExpr(vector<unique_ptr<Expr>> elems, shared_ptr<ValueType> type)
        : elements(std::move(elems)), type(type) {}
};

struct ParenExpr: public Expr {
    unique_ptr<Expr> expr;

    shared_ptr<ValueType> getType() const { return expr->getType(); }
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    ParenExpr(unique_ptr<Expr> expr)
        : expr(std::move(expr)) {}
};

struct IndexExpr: public Expr {
    vector<unique_ptr<Expr>> Idxs;
    shared_ptr<ValueType> type;
    string name;

    shared_ptr<ValueType> getType() const { return type; }
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    IndexExpr(const string& name, vector<unique_ptr<Expr>> Idxs, shared_ptr<ValueType> type)
        : name(name), Idxs(std::move(Idxs)), type(type) {}
};


// Statements

struct WarStmt: public Stmt {
    shared_ptr<ValueType> type;
    unique_ptr<Expr> value;
    string name;

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }
    
    WarStmt(const string& name, unique_ptr<Expr> value, shared_ptr<ValueType> type)
        : name(name), value(std::move(value)), type(type) {}
};

struct TrenStmt: public Stmt {
    vector<pair<string, shared_ptr<ValueType>>> args;
    shared_ptr<ValueType> retType;
    unique_ptr<Stmt> func_body;
    string name;

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }
    
    TrenStmt(const string& name, unique_ptr<Stmt> fb, shared_ptr<ValueType> type, vector<pair<string, shared_ptr<ValueType>>> args)
        : name(name), func_body(std::move(fb)), retType(type), args(std::move(args)) {}
};


struct RetStmt: public Stmt {
    unique_ptr<Expr> expr;

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    RetStmt(unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

struct IfStmt: public Stmt {
    unique_ptr<Expr> Cond;
    unique_ptr<Stmt> Body;

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    IfStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body)
        : Cond(std::move(cond)), Body(std::move(body)) {}
};

struct AliveStmt: public Stmt {
    unique_ptr<Expr> Cond;
    unique_ptr<Stmt> Body;

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    AliveStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body)
        : Cond(std::move(cond)), Body(std::move(body)) {}
};

struct HighExpr: public Stmt {
    unique_ptr<Expr> expr;
    
    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }
    
    HighExpr(unique_ptr<Expr> expr)
        : expr(std::move(expr)) {}
};

struct ParenStmts: public Stmt {
    vector<unique_ptr<Stmt>> stmts;

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    ParenStmts(vector<unique_ptr<Stmt>> stmts)
        : stmts(std::move(stmts)) {}
};

struct Input: public Node {
    vector<unique_ptr<Stmt>> stmts;

    Value *accept(ASTVisitor& visitor) { return visitor.visit(*this); }

    Input(vector<unique_ptr<Stmt>> stmts)
        : stmts(std::move(stmts)) {}
};
