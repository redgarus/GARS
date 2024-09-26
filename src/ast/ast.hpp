#ifndef AST_H
#define AST_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "../lexer/lexer.hpp"

using llvm::Value, llvm::Function, llvm::AllocaInst;

namespace AST {
    
using std::unique_ptr, std::shared_ptr, std::make_unique, std::make_shared;
using std::string, std::to_string, std::vector, std::unordered_map;
using ld = long double;
using ll = long long;
using lexer::TOKEN;

//* Types
enum class Type {
    String, Bool, Int, Null
};

Type getDType(TOKEN::lexeme);

//* Sym Table and Sym
class Symbol {
public:
    enum SymType {
        WAR, FUNCTION
    };
private:
    // AllocaInst *alloc = nullptr;
    string name;
    Type Vtype;
    SymType sym_type;
public:
    SymType getSymType() const;
    void setAlloc(AllocaInst *);
    AllocaInst *getAlloc() const;
    const string& getName() const;
    Type getType() const;
    Symbol(const string&, Type, SymType);
};

class Table {
    unordered_map<string, shared_ptr<Symbol>> symbols;
    shared_ptr<Table> Prev;
public:
    shared_ptr<Table> getPrev();
    void addSym(const string&, shared_ptr<Symbol>);
    shared_ptr<Symbol> getSym(const string&);

    Table(shared_ptr<Table>);
};

//* Interfaces
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual bool hasRet() const;
    virtual Value *codegen() = 0;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual Type getType() const = 0;
    virtual bool lvalue() const;
    virtual Value *codegen() = 0;
};

class Lvalue: public Expr {
public:
    virtual const string& getName() const = 0;
    virtual shared_ptr<Table> getTable() = 0;
    bool lvalue() const override;
};

//* Input
class Input {
    vector<unique_ptr<Stmt>> stmts;
public:
    Value *codegen();
    Input(vector<unique_ptr<Stmt>>);
};

//* Expressions

// 1 level
class AssignExpr: public Expr {
    unique_ptr<Lvalue> LHS;
    unique_ptr<Expr> RHS;
    Type Vtype;
public:
    Value *codegen() override;
    Type getType() const override;
    AssignExpr(unique_ptr<Lvalue>, unique_ptr<Expr>, Type);
};

// 2 level
class BoolExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    Type Vtype;
    string Op;
public:
    Value *codegen() override;
    Type getType() const override;
    BoolExpr(const string&, unique_ptr<Expr>, unique_ptr<Expr>, Type);
};

class AddExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    Type Vtype;
    string Op;
public:
    Value *codegen() override;
    Type getType() const override;
    AddExpr(const string&, unique_ptr<Expr>, unique_ptr<Expr>, Type);
};

class TermExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    Type Vtype;
    string Op;
public:
    Value *codegen() override;
    Type getType() const override;
    TermExpr(const string&, unique_ptr<Expr>, unique_ptr<Expr>, Type);
};

// 3 level
class IntExpr: public Expr {
    ll value;
public:
    Value *codegen() override;
    Type getType() const override;
    IntExpr(ll);
};

class TrueExpr: public Expr {
    int value;
public:
    Value *codegen() override;
    Type getType() const override;
    TrueExpr(int);
};

class StrExpr: public Expr {
    string value;
public:
    Value *codegen() override;
    Type getType() const override;
    StrExpr(const string&);
};

class IDExpr: public Lvalue {
    shared_ptr<Table> CurrTable;
    string name;
    Type Vtype;
public:
    Value *codegen() override;
    Type getType() const override;
    const string& getName() const override;
    shared_ptr<Table> getTable() override;
    IDExpr(const string&, shared_ptr<Table>, Type);
};

class CallExpr: public Expr {
    vector<unique_ptr<Expr>> args;
    shared_ptr<Table> CurrTable;
    string name;
    Type Vtype;
public:
    Value *codegen() override;
    Type getType() const override;
    CallExpr(vector<unique_ptr<Expr>>, shared_ptr<Table>, const string&, Type);
};

class ParenExpr: public Expr {
    unique_ptr<Expr> expr;
public:
    Value *codegen() override;
    Type getType() const override;
    ParenExpr(unique_ptr<Expr>);
};

//* Statements

class HighExpr: public Stmt {
    unique_ptr<Expr> expr;
public:
    Value *codegen() override;
    HighExpr(unique_ptr<Expr>);
};

class TrenStmt: public Stmt {
    shared_ptr<Table> CurrTable;
    unique_ptr<Stmt> body;
    vector<string> args;
    string name;
    Type Vtype;

public:
    Function *codegen() override;
    shared_ptr<Table> getTable();
    const string& getName() const;
    TrenStmt(const string&, unique_ptr<Stmt>, vector<string>, shared_ptr<Table>, Type);
};

class WarStmt: public Stmt {
    vector<unique_ptr<AssignExpr>> wars;
    shared_ptr<Table> CurrTable;
public:
    Value *codegen() override;
    WarStmt(vector<unique_ptr<AssignExpr>>, shared_ptr<Table>);
};

class RetStmt: public Stmt {
    unique_ptr<Expr> expr;
public:
    Value *codegen() override;
    bool hasRet() const override;
    RetStmt(unique_ptr<Expr>);
};

class ParenStmts: public Stmt {
    vector<unique_ptr<Stmt>> stmts;
    bool hRet;
    // shared_ptr<Table> CurrTable;
public:
    Value *codegen() override;
    bool hasRet() const override;
    ParenStmts(vector<unique_ptr<Stmt>>, bool);
};

class IfStmt: public Stmt {
    unique_ptr<Expr> Cond;
    unique_ptr<Stmt> Body;
public:
    Value *codegen() override;
    IfStmt(unique_ptr<Expr>, unique_ptr<Stmt>);
};

class AliveStmt: public Stmt {
    unique_ptr<Expr> Cond;
    unique_ptr<Stmt> Body;
public:
    Value *codegen() override;
    AliveStmt(unique_ptr<Expr>, unique_ptr<Stmt>);
};

}
#endif
