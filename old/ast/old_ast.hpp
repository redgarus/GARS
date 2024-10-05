#ifndef AST_H
#define AST_H

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"
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

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../lexer/lexer.hpp"

using llvm::Value, llvm::Function, llvm::AllocaInst, llvm::Module;

namespace AST {
    
using std::unique_ptr, std::shared_ptr;
using std::string, std::vector, std::unordered_map;
using ld = long double;
using ll = long long;
using lexer::TOKEN;

void MainPool();
void CreateRet();
unique_ptr<Module> getModule();
int GenerateObjFile(std::string);
//* Types

class Type {
public:
    enum TypeWord {
        Array, Int, Null
    };

    virtual shared_ptr<Type> getElementType() const;
    virtual TypeWord get() const = 0; 
    virtual ~Type() = default;
};

class IntType: public Type {
public:
    TypeWord get() const override;
};

class ArrayType: public Type {
    shared_ptr<Type> elem_type;
public:
    shared_ptr<Type> getElementType() const override;
    TypeWord get() const override;
    
    ArrayType(shared_ptr<Type>);
};

Type::TypeWord getDType(TOKEN::lexeme);

//* Sym Table and Sym
class Symbol {
public:
    enum SymType {
        WAR, FUNCTION
    };
private:
    AllocaInst *alloc = nullptr;
    Function *func = nullptr;
    vector<shared_ptr<Type>> arg_types;
    shared_ptr<Type> Vtype;
    SymType sym_type;
    string name;
public:
    SymType getSymType() const;
    AllocaInst *getAlloc() const;
    Function *getFunction() const;
    const string& getName() const;
    shared_ptr<Type> getType() const;
    const vector<shared_ptr<Type>>& getArgs() const;

    void setArgs(vector<shared_ptr<Type>>);
    void setFunction(Function *);
    void setAlloc(AllocaInst *);

    Symbol(const string&, shared_ptr<Type>, SymType);
};

class Table {
    unordered_map<string, shared_ptr<Symbol>> symbols;
    shared_ptr<Table> Prev;
public:
    shared_ptr<Table> getPrev();
    shared_ptr<Symbol> getSym(const string&);
    void addSym(const string&, shared_ptr<Symbol>);

    Table(shared_ptr<Table>);
};

//* Interfaces
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual shared_ptr<Type> hasRet() const;
    virtual Value *codegen() = 0;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual shared_ptr<Type> getType() const = 0;
    virtual bool lvalue() const;
    virtual Value *codegen() = 0;
    virtual int size();
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
    shared_ptr<Type> Vtype;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    AssignExpr(unique_ptr<Lvalue>, unique_ptr<Expr>, shared_ptr<Type>);
};

// 2 level
class BoolExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    shared_ptr<Type> Vtype;
    string Op;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    BoolExpr(const string&, unique_ptr<Expr>, unique_ptr<Expr>, shared_ptr<Type>);
};

class AddExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    shared_ptr<Type> Vtype;
    string Op;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    AddExpr(const string&, unique_ptr<Expr>, unique_ptr<Expr>, shared_ptr<Type>);
};

class TermExpr: public Expr {
    unique_ptr<Expr> LHS, RHS;
    shared_ptr<Type> Vtype;
    string Op;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    TermExpr(const string&, unique_ptr<Expr>, unique_ptr<Expr>, shared_ptr<Type>);
};

// 3 level
class IntExpr: public Expr {
    shared_ptr<Type> Vtype;
    ll value;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    IntExpr(ll);
};

class ArrayExpr: public Expr {
    vector<unique_ptr<Expr>> value;
    shared_ptr<Type> Vtype;
public:
    int size() override;
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    ArrayExpr(vector<unique_ptr<Expr>>, shared_ptr<Type>);
};

class IDExpr: public Lvalue {
    shared_ptr<Table> CurrTable;
    string name;
    shared_ptr<Type> Vtype;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    const string& getName() const override;
    shared_ptr<Table> getTable() override;
    IDExpr(const string&, shared_ptr<Table>, shared_ptr<Type>);
};

class CallExpr: public Expr {
    vector<unique_ptr<Expr>> args;
    shared_ptr<Table> CurrTable;
    string name;
    shared_ptr<Type> Vtype;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
    CallExpr(vector<unique_ptr<Expr>>, shared_ptr<Table>, const string&, shared_ptr<Type>);
};

class ParenExpr: public Expr {
    unique_ptr<Expr> expr;
public:
    Value *codegen() override;
    shared_ptr<Type> getType() const override;
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
    shared_ptr<Type> Vtype;
    unique_ptr<Stmt> body;
    vector<string> args;
    string name;

public:
    Function *codegen() override;
    shared_ptr<Table> getTable();
    const string& getName() const;
    TrenStmt(const string&, unique_ptr<Stmt>, vector<string>, shared_ptr<Table>, shared_ptr<Type>);
};

class WarStmt: public Stmt {
    vector<std::pair<string, unique_ptr<Expr>>> wars;
    shared_ptr<Table> CurrTable;
public:
    Value *codegen() override;
    WarStmt(vector<std::pair<string, unique_ptr<Expr>>>, shared_ptr<Table>);
};

class RetStmt: public Stmt {
    unique_ptr<Expr> expr;
public:
    Value *codegen() override;
    shared_ptr<Type> hasRet() const override;
    RetStmt(unique_ptr<Expr>);
};

class ParenStmts: public Stmt {
    vector<unique_ptr<Stmt>> stmts;
    shared_ptr<Type> RetType;
    // shared_ptr<Table> CurrTable;
public:
    Value *codegen() override;
    shared_ptr<Type> hasRet() const override;
    ParenStmts(vector<unique_ptr<Stmt>>, shared_ptr<Type>);
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
