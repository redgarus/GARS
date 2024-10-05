#pragma once

#include <memory>
#include <string>
#include <vector>

#include "type.hpp"
#include "token.hpp"
#include "table.hpp"

using std::shared_ptr, std::unique_ptr, std::make_unique, std::make_shared, std::string, std::vector;

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
class ParenExpr;

class Input;

class ASTVisitor {
public:
    virtual void visit(WarStmt&) = 0;
    virtual void visit(TrenStmt&) = 0;
    virtual void visit(RetStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(AliveStmt&) = 0;
    virtual void visit(HighExpr&) = 0;
    virtual void visit(ParenStmts&) = 0;

    virtual void visit(AssignExpr&) = 0;
    virtual void visit(BoolExpr&) = 0;
    virtual void visit(AddExpr&) = 0;
    virtual void visit(TermExpr&) = 0;
    virtual void visit(IDExpr&) = 0;
    virtual void visit(CallExpr&) = 0;
    virtual void visit(IntExpr&) = 0;
    virtual void visit(ArrayExpr&) = 0;
    virtual void visit(ParenExpr&) = 0;

    virtual void visit(Input&) = 0;
};

class Node {
    virtual void accept(shared_ptr<ASTVisitor>) = 0;
};

class Stmt {
public:
    virtual ~Stmt() = default;
};

class Expr {
public:
    virtual shared_ptr<ValueType> getType() const = 0;
    virtual ~Expr() = default;
};

class AssignExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    shared_ptr<ValueType> type;
public:
    shared_ptr<ValueType> getType() const { return type; }
    
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    AssignExpr(unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> Tval)
        : LHS(std::move(lhs)), RHS(std::move(rhs)), type(Tval) {}
};


class BoolExpr: public Expr {
    shared_ptr<ValueType> type;
    unique_ptr<Expr> LHS, RHS;
    TOKEN::lexeme OP;    
public:
    shared_ptr<ValueType> getType() const { return type; }
    
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }
    
    BoolExpr(TOKEN::lexeme OP, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> Tval)
        : OP(OP), LHS(std::move(lhs)), RHS(std::move(rhs)), type(Tval) {}
};


class AddExpr: public Expr {
    shared_ptr<ValueType> type;
    unique_ptr<Expr> LHS, RHS;
    TOKEN::lexeme OP;
public:
    shared_ptr<ValueType> getType() const { return type; }
    
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }
    
    AddExpr(TOKEN::lexeme OP, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> Tval)
        : OP(OP), LHS(std::move(lhs)), RHS(std::move(rhs)), type(Tval) {}
};

class TermExpr: public Expr {
    shared_ptr<ValueType> type;
    unique_ptr<Expr> LHS, RHS;
    TOKEN::lexeme OP;
public:
    shared_ptr<ValueType> getType() const { return type; }
    
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }
    
    TermExpr(TOKEN::lexeme OP, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<ValueType> Tval)
        : OP(OP), LHS(std::move(lhs)), RHS(std::move(rhs)), type(Tval) {}
};


class IDExpr: public Expr {
    shared_ptr<ValueType> type;
    shared_ptr<Table> scope;
    string name;
public:
    shared_ptr<ValueType> getType() const { return type; }
    const string& getName() const { return name; }
    
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }
    
    IDExpr(const string& name, shared_ptr<Table> scope, shared_ptr<ValueType> type)
        : name(name), scope(scope), type(type) {}
};


class CallExpr: public Expr {
    vector<unique_ptr<Expr>> args;
    shared_ptr<ValueType> type;
    shared_ptr<Table> scope;
    string name;
public:
    shared_ptr<ValueType> getType() const { return type; }
    const string& getName() const { return name; }
    
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    CallExpr(const string& name, shared_ptr<Table> scope, shared_ptr<ValueType> type, vector<unique_ptr<Expr>> args)
        : name(name), scope(scope), type(type), args(std::move(args)) {}
};

class IntExpr: public Expr {
    shared_ptr<ValueType> type;
    ll value;
   
public:
    shared_ptr<ValueType> getType() const { return type; }

    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    IntExpr(ll val) : value(val), type(make_shared<IntType>()) {}
};

class ArrayExpr: public Expr {
    vector<unique_ptr<Expr>> elements;
    shared_ptr<ValueType> type;
public:
    shared_ptr<ValueType> getType() const { return type; }

    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    ArrayExpr(vector<unique_ptr<Expr>> elems, shared_ptr<ValueType> type)
        : elements(std::move(elems)), type(type) {}
};


class WarStmt: public Stmt {
    shared_ptr<Table> scope;
    unique_ptr<Expr> value;
    string name;
public:
    const string& getName() const { return name; }
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }
    
    WarStmt(const string& name, unique_ptr<Expr> value, shared_ptr<Table> tab)
        : scope(tab), name(name), value(std::move(value)) {}
};

class TrenStmt: public Stmt {
    vector<shared_ptr<ValueType>> args_types;
    shared_ptr<ValueType> type;
    shared_ptr<Table> scope;
    vector<string> args_names;
    string name;
public:
    const string& getName() const { return name; }
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }
    
    TrenStmt(const string& name, vector<shared_ptr<ValueType>> args_types, shared_ptr<ValueType> type, shared_ptr<Table> scope, vector<string> args_names)
        : name(name), args_types(args_types), args_names(args_names), type(type), scope(scope) {}
};


class RetStmt: public Stmt {
    unique_ptr<Expr> expr;
public:
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    RetStmt(unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

class IfStmt: public Stmt {
    unique_ptr<Expr> Cond;
    unique_ptr<Stmt> Body;
public:
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    IfStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body)
        : Cond(std::move(cond)), Body(std::move(body)) {}
};

class AliveStmt: public Stmt {
    unique_ptr<Expr> Cond;
    unique_ptr<Stmt> Body;
public:
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    AliveStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body)
        : Cond(std::move(cond)), Body(std::move(body)) {}
};

class HighExpr: public Stmt {
    unique_ptr<Expr> expr;
public:
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }
    
    HighExpr(unique_ptr<Expr> expr)
        : expr(std::move(expr)) {}
};

class ParenStmts: public Stmt {
    vector<unique_ptr<Stmt>> stmts;
public:
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    ParenStmts(vector<unique_ptr<Stmt>> stmts)
        : stmts(std::move(stmts)) {}
};

class Input: public Node {
    vector<unique_ptr<Stmt>> stmts;
public:
    void accept(shared_ptr<ASTVisitor> visitor) { visitor->visit(*this); }

    Input(vector<unique_ptr<Stmt>> stmts)
        : stmts(std::move(stmts)) {}
};

class LVisitor: ASTVisitor {
public:
    bool lvalue;
    string name;

    void visit(WarStmt& node) override { lvalue = false; name = node.getName(); }
    void visit(TrenStmt& node) override { lvalue = false; name = node.getName(); }
    void visit(RetStmt& node) override { lvalue = false; }
    void visit(IfStmt& node) override { lvalue = false; }
    void visit(AliveStmt& node) override { lvalue = false; }
    void visit(HighExpr& node) override { lvalue = false; }

    void visit(ParenStmts& node) override { lvalue = false; }
    void visit(AssignExpr& node) override { lvalue = false; }
    void visit(BoolExpr& node) override { lvalue = false; }
    void visit(AddExpr& node) override { lvalue = false; }
    void visit(TermExpr& node) override { lvalue = false; }
    void visit(IDExpr& node) override { lvalue = true; name = node.getName(); }
    void visit(CallExpr& node) override { lvalue = false; name = node.getName(); }
    void visit(IntExpr& node) override { lvalue = false; }
    void visit(ArrayExpr& node) override { lvalue = false; }
    void visit(ParenExpr& node) override { lvalue = false; }

    void visit(Input& node) override { lvalue = false; }
};
