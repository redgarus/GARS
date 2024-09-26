#include "ast.hpp"

namespace AST {

bool Stmt::hasRet() const { return false; }
bool ParenStmts::hasRet() const { return hRet; }
bool RetStmt::hasRet() const { return true; }

//* getType definitions
Type Symbol::getType() const { return Vtype; }
Type AssignExpr::getType() const { return Vtype; }
Type BoolExpr::getType() const { return Vtype; }
Type AddExpr::getType() const { return Vtype; }
Type TermExpr::getType() const { return Vtype; }

Type IDExpr::getType() const { return Vtype; }

Type StrExpr::getType() const { return Type::String; }
Type IntExpr::getType() const { return Type::Int; }
Type TrueExpr::getType() const { return Type::Int; }
Type CallExpr::getType() const { return Vtype; };
Type ParenExpr::getType() const { return expr->getType(); }


//* Constructors definitions
Symbol::Symbol(const string& name, Type type, SymType sym_type) : name(name), Vtype(type), sym_type(sym_type)  {}
Table::Table(shared_ptr<Table> prev) : Prev(prev) {}

Input::Input(vector<unique_ptr<Stmt>> stmts) : stmts(std::move(stmts)) {}

// Exprs
AssignExpr::AssignExpr(unique_ptr<Lvalue> lhs, unique_ptr<Expr> rhs, Type type) : LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}
BoolExpr::BoolExpr(const string& Op, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, Type type) : Op(Op), LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}
AddExpr::AddExpr(const string& Op, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, Type type) : Op(Op), LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}
TermExpr::TermExpr(const string& Op, unique_ptr<Expr> lhs, unique_ptr<Expr> rhs, Type type) : Op(Op), LHS(std::move(lhs)), RHS(std::move(rhs)), Vtype(type) {}

IntExpr::IntExpr(ll val) : value(val) {}
TrueExpr::TrueExpr(int val) : value(val) {}
StrExpr::StrExpr(const string& val) : value(val) {}
IDExpr::IDExpr(const string& name, shared_ptr<Table> tab, Type type) : CurrTable(tab), name(name), Vtype(type) {}
CallExpr::CallExpr(vector<unique_ptr<Expr>> args, shared_ptr<Table> tab, const string& name, Type type) : args(std::move(args)), CurrTable(tab), name(name), Vtype(type) {}
ParenExpr::ParenExpr(unique_ptr<Expr> expr) : expr(std::move(expr)) {}

// Stmts
HighExpr::HighExpr(unique_ptr<Expr> expr) : expr(std::move(expr)) {}
TrenStmt::TrenStmt(const string& name, unique_ptr<Stmt> bod, vector<string> args, shared_ptr<Table> tab, Type type) : name(name), body(std::move(bod)), args(std::move(args)), Vtype(type) {}
WarStmt::WarStmt(vector<unique_ptr<AssignExpr>> wars, shared_ptr<Table> tab) : wars(std::move(wars)), CurrTable(tab) {}
RetStmt::RetStmt(unique_ptr<Expr> expr) : expr(std::move(expr)) {}
ParenStmts::ParenStmts(vector<unique_ptr<Stmt>> stmts, bool hret) : stmts(std::move(stmts)), hRet(hret) {}
IfStmt::IfStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body) : Cond(std::move(cond)), Body(std::move(body)) {}
AliveStmt::AliveStmt(unique_ptr<Expr> cond, unique_ptr<Stmt> body) : Cond(std::move(cond)), Body(std::move(body)) {}

//* lvalue definition
bool Expr::lvalue() const { return false; }
bool Lvalue::lvalue() const { return true; }

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

//* getDType
Type getDType(TOKEN::lexeme tok) {
    switch(tok) {
        case TOKEN::STRINGTYPE: return Type::String;
        case TOKEN::BOOLTYPE:
        case TOKEN::INTTYPE: return Type::Int;
        default: return Type::Null;
    }
}

//* get Sym Type
Symbol::SymType Symbol::getSymType() const {
    return sym_type;
}
    
}
