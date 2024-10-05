#include "ast.hpp"
#include <llvm-18/llvm/IR/Instructions.h>

using std::make_shared;

namespace AST {

shared_ptr<Type> Stmt::hasRet() const { return nullptr; }
shared_ptr<Type> ParenStmts::hasRet() const { return RetType; }
shared_ptr<Type> RetStmt::hasRet() const { return expr->getType(); }

//* getType definitions
shared_ptr<Type> Symbol::getType() const { return Vtype; }
shared_ptr<Type> AssignExpr::getType() const { return Vtype; }
shared_ptr<Type> BoolExpr::getType() const { return Vtype; }
shared_ptr<Type> AddExpr::getType() const { return Vtype; }
shared_ptr<Type> TermExpr::getType() const { return Vtype; }

shared_ptr<Type> IDExpr::getType() const { return Vtype; }

shared_ptr<Type> ArrayExpr::getType() const { return Vtype; }
shared_ptr<Type> IntExpr::getType() const { return Vtype; }

shared_ptr<Type> CallExpr::getType() const { return Vtype; };
shared_ptr<Type> ParenExpr::getType() const { return expr->getType(); }


//* Constructors definitions
Symbol::Symbol(const string& name, shared_ptr<Type> type, SymType sym_type) : name(name), Vtype(type), sym_type(sym_type)  {}
Table::Table(shared_ptr<Table> prev) : Prev(prev) {}

Input::Input(vector<unique_ptr<Stmt>> stmts) : stmts(std::move(stmts)) {}

// Exprs
AssignExpr::AssignExpr(unique_ptr<Lvalue> lhs, unique_ptr<Expr> rhs, shared_ptr<Type> type) : LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}
BoolExpr::BoolExpr(const string& Op, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<Type> type) : Op(Op), LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}
AddExpr::AddExpr(const string& Op, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<Type> type) : Op(Op), LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}
TermExpr::TermExpr(const string& Op, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, shared_ptr<Type> type) : Op(Op), LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}

IntExpr::IntExpr(ll val) : value(val), Vtype(make_shared<IntType>()) {}
ArrayExpr::ArrayExpr(vector<unique_ptr<Expr>> val, shared_ptr<Type> type) : value(std::move(val)), Vtype(type) {}
IDExpr::IDExpr(const string& name, shared_ptr<Table> tab, shared_ptr<Type> type) : CurrTable(tab), name(name), Vtype(type) {}
CallExpr::CallExpr(vector<unique_ptr<Expr>> args, shared_ptr<Table> tab, const string& name, shared_ptr<Type> type) : args(std::move(args)), CurrTable(tab), name(name), Vtype(type) {}
ParenExpr::ParenExpr(unique_ptr<Expr> expr) : expr(std::move(expr)) {}

// Stmts
HighExpr::HighExpr(unique_ptr<Expr> expr) : expr(std::move(expr)) {}
TrenStmt::TrenStmt(const string& name, unique_ptr<Stmt> bod, vector<string> args, shared_ptr<Table> tab, shared_ptr<Type> type) : name(name), body(std::move(bod)), args(std::move(args)), Vtype(type) {}
WarStmt::WarStmt(vector<std::pair<string, unique_ptr<Expr>>> wars, shared_ptr<Table> tab) : wars(std::move(wars)), CurrTable(tab) {}
RetStmt::RetStmt(unique_ptr<Expr> expr) : expr(std::move(expr)) {}
ParenStmts::ParenStmts(vector<unique_ptr<Stmt>> stmts, shared_ptr<Type> RetType) : stmts(std::move(stmts)), RetType(RetType) {}
IfStmt::IfStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body) : Cond(std::move(cond)), Body(std::move(body)) {}
AliveStmt::AliveStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body) : Cond(std::move(cond)), Body(std::move(body)) {}

// Types
ArrayType::ArrayType(shared_ptr<Type> elem_type) : elem_type(elem_type) {}

//* lvalue definition
int Expr::size() { return 0; }
bool Expr::lvalue() const { return false; }
bool Lvalue::lvalue() const { return true; }
int ArrayExpr::size() { return value.size(); }

//* Table
shared_ptr<Table> Table::getPrev() { return Prev; }
void Table::addSym(const string& name, shared_ptr<Symbol> sym) { symbols[name] = sym; }
shared_ptr<Symbol> Table::getSym(const string& name) { 
    if(symbols.count(name))
        return symbols[name];
    else if(Prev)
        return Prev->getSym(name);
    else
        return nullptr;
}

//* getnames & gettables
const string& IDExpr::getName() const { return name; }
shared_ptr<Table> IDExpr::getTable() { return CurrTable; }
const string& TrenStmt::getName() const { return name; }
shared_ptr<Table> TrenStmt::getTable() { return CurrTable; }

//* Types

shared_ptr<Type> Type::getElementType() const {
    return nullptr;
}

Type::TypeWord IntType::get() const {
    return Type::Int;
}

Type::TypeWord ArrayType::get() const {
    return Type::Array;
}

shared_ptr<Type> ArrayType::getElementType() const {
    return elem_type;
}

Type::TypeWord getDType(TOKEN::lexeme tok) {
    switch(tok) {
        case TOKEN::ARRAYTYPE: return Type::Array;
        case TOKEN::INTTYPE: return Type::Int;
        default: return Type::Null;
    }
}

//* Symbol methods
Symbol::SymType Symbol::getSymType() const {
    return sym_type;
}
    
void Symbol::setArgs(vector<shared_ptr<Type>> args) {
    arg_types = std::move(args);
}

const vector<shared_ptr<Type>>& Symbol::getArgs() const {
    return arg_types;
}

Function *Symbol::getFunction() const {
    return func;
}

void Symbol::setFunction(Function *function) {
    func = function;
}

AllocaInst *Symbol::getAlloc() const {
    return alloc;
}

void Symbol::setAlloc(AllocaInst *al) {
    alloc = al;
}

}
