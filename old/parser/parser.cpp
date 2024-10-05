#include "parser.hpp"

namespace Parser {

#define debug_tok(toke) std::cout << "TOKEN: " << toke.tok << ". Line: " << toke.line << ". Word: " << toke.word << ".\n";

using std::make_unique, std::make_shared;    
using lexer::getNextToken;
using std::cerr;

// Types

bool checkTypes(shared_ptr<Type> E1, shared_ptr<Type> E2) {
    if(E1->getElementType() && E2->getElementType()) {
        
        while(E1->getElementType() && E2->getElementType() && E2->get() == E2->get()) {
            E1 = E1->getElementType();
            E2 = E2->getElementType();
        }
        
        if(E1 && E2 && E1->get() == E2->get())
            return true;
    }
    else if(E1->get() == E2->get())
        return true;

    return false;
}

//* lexer and table block
static TOKEN CurrTok;
static TOKEN nextToken() {
    return CurrTok = getNextToken();
}

static shared_ptr<Table> CTable;
static shared_ptr<Table> prevTable() { 
    return CTable = CTable->getPrev();
}
static shared_ptr<Table> nextTable() {
    return CTable = make_shared<Table>(CTable? CTable:nullptr);
}

//* Log Error Block
void LogError(const string& msg) {
    cerr << "Syntax Error: " << msg << ". Line: " << CurrTok.line << '\n';
}

unique_ptr<Input> LogInputError(const string& msg) {
    LogError(msg);
    return nullptr;
}

unique_ptr<Stmt> LogStmtError(const string& msg) {
    LogError(msg);
    return nullptr;
}

unique_ptr<Expr> LogExprError(const string& msg) {
    LogError(msg);
    return nullptr;
}

shared_ptr<Type> LogTypeError(const string& msg) {
    LogError(msg);
    return nullptr;
}

//* Parser Block
unique_ptr<Input> ParseInput() {
    nextToken();
    nextTable();

    std::vector<shared_ptr<Type>> args_types{ make_shared<IntType>() };

    print_sym->setArgs(std::move(args_types));
    
    CTable->addSym("print", print_sym);

    vector<unique_ptr<Stmt>> stmts;

    while(CurrTok.tok != TOKEN::EOFILE) {
        unique_ptr<Stmt> stmt = ParseStatement();

        if(!stmt)
            return nullptr;

        stmts.push_back(std::move(stmt));
    }

    return make_unique<Input>(std::move(stmts));
}

static unique_ptr<Stmt> ParseStatement() {
    switch(CurrTok.tok) {
        case TOKEN::IF: return ParseIfStmt();
        case TOKEN::WAR: return ParseWarStmt();
        case TOKEN::TREN: return ParseTrenStmt();
        case TOKEN::ALIVE: return ParseAliveStmt();
        case TOKEN::RETURN: return ParseRetStmt();
        case TOKEN::LBRA: return ParseParenStmts();
        case TOKEN::EOFILE: return LogStmtError("missing statement");
        default: return ParseHighExpr();
    }

    return nullptr;
}

static unique_ptr<Stmt> ParseIfStmt() {
    nextToken(); // eat if
    if(CurrTok.tok != TOKEN::LBAR)
        return LogStmtError("excepted '('");

    nextToken(); // eat [
    unique_ptr<Expr> cond = ParseExpression();

    if(!cond)
        return nullptr;

    if(cond->getType()->get() != Type::Int)
        return LogStmtError("invalid type for cond");
    
    if(CurrTok.tok != TOKEN::RBAR)
        return LogStmtError("excepted ')'");

    nextToken();
    if(CurrTok.tok != TOKEN::DO)
        return LogStmtError("excepted do-token");

    nextToken();        
    unique_ptr<Stmt> body = ParseStatement();

    if(!body)
        return nullptr;
    
    return make_unique<IfStmt>(std::move(cond), std::move(body));
}

static unique_ptr<Stmt> ParseAliveStmt() {
    nextToken();
    if(CurrTok.tok != TOKEN::BY)
        return LogStmtError("excepted by-token");
    
    nextToken();
    if(CurrTok.tok != TOKEN::LBRACE)
        return LogStmtError("excepted '['");
    
    nextToken();
    unique_ptr<Expr> cond = ParseExpression();

    if(!cond)
        return nullptr;
    
    if(cond->getType()->get() != Type::Int)
        return LogStmtError("invalid type for cond");

    if(CurrTok.tok != TOKEN::RBRACE)
        return LogStmtError("excepted ']'");

    nextToken();
    unique_ptr<Stmt> body = ParseStatement();

    if(!body)
        return nullptr;
    
    return make_unique<AliveStmt>(std::move(cond), std::move(body));
}

static unique_ptr<Stmt> ParseWarStmt() {
    vector<std::pair<string, unique_ptr<Expr>>> wars;

    nextToken();
    while(CurrTok.tok != TOKEN::SEMICOL) {
        if(CurrTok.tok != TOKEN::IDENTIFIER)
            return LogStmtError("invalid identifier for war");

        string warName = CurrTok.word;

        nextToken();
        if(CurrTok.tok != TOKEN::COL) 
            return LogStmtError("excepted ':'");

        nextToken();
        shared_ptr<Type> type = ParseType();
        if(!type)
            return LogStmtError("undefined type");
        
        if(CurrTok.tok != TOKEN::ASSIGN)
            return LogStmtError("excepted '='");
        
        nextToken();
        unique_ptr<Expr> valu = ParseExpression();

        if(!valu)
            return nullptr;

        if(!checkTypes(valu->getType(), type))
            return LogStmtError("invalid expr type for war");

        wars.push_back({warName, std::move(valu)});
        CTable->addSym(warName, make_shared<Symbol>(warName, type, Symbol::WAR));

        if(CurrTok.tok != TOKEN::SEMICOL && CurrTok.tok != TOKEN::COMMA)
            return LogStmtError("excepted ';'");
        else if(CurrTok.tok == TOKEN::COMMA) {
            nextToken();
            if(CurrTok.tok == TOKEN::SEMICOL)
                return LogStmtError("excepted assign expr");
        }
    }
    
    nextToken(); // eat semicol

    return make_unique<WarStmt>(std::move(wars), CTable);
}

static unique_ptr<Stmt> ParseTrenStmt() {
    nextToken();
    shared_ptr<Type> func_type = ParseType();
    if(!func_type)
        return LogStmtError("invalid func type");
    
    if(CurrTok.tok != TOKEN::IDENTIFIER)
        return LogStmtError("excepted identifier");
    
    string func_name = CurrTok.word;

    nextToken();
    if(CurrTok.tok != TOKEN::LBRACE)
        return LogStmtError("excepted '['");

    nextToken();

    shared_ptr<Table> func_tab = nextTable();

    vector<string> args;
    vector<shared_ptr<Type>> args_types;
    while(CurrTok.tok != TOKEN::RBRACE) {
        shared_ptr<Type> type = ParseType();
        if(!type)
            return LogStmtError("invalid type");
        
        if(CurrTok.tok != TOKEN::IDENTIFIER)
            return LogStmtError("excepted arg identifier");
        
        string name = CurrTok.word;
    
        CTable->addSym(name, make_shared<Symbol>(name, type, Symbol::WAR));

        args.push_back(name);
        args_types.push_back(type);

        nextToken();
        if(CurrTok.tok != TOKEN::RBRACE && CurrTok.tok != TOKEN::COMMA)
            return LogStmtError("excepted ']'");
        else if(CurrTok.tok == TOKEN::COMMA) {
            nextToken();
            if(CurrTok.tok == TOKEN::RBRACE)
                return LogStmtError("excepted argument");
        }
    }

    shared_ptr<Symbol> func_sym = make_shared<Symbol>(func_name, func_type, Symbol::FUNCTION);

    func_sym->setArgs(std::move(args_types));
    
    CTable->addSym(func_name, func_sym);

    nextToken();
    unique_ptr<Stmt> body = ParseStatement();

    if(!body)
        return nullptr;
    
    if(!checkTypes(body->hasRet(), func_type))
        return LogStmtError("invalid return type in function");

    prevTable();

    return make_unique<TrenStmt>(func_name, std::move(body), std::move(args), func_tab, func_type);
}

static unique_ptr<Stmt> ParseHighExpr() {
    unique_ptr<Expr> expr = ParseExpression();
    if(!expr)
        return nullptr;
    
    if(CurrTok.tok != TOKEN::SEMICOL)
        return LogStmtError("excepted ';'");

    nextToken();

    return make_unique<HighExpr>(std::move(expr));
}

static unique_ptr<Stmt> ParseParenStmts() {
    vector<unique_ptr<Stmt>> stmts;
    shared_ptr<Type> retType = nullptr;

    nextTable();
    nextToken();
    while(CurrTok.tok != TOKEN::RBRA) {
        unique_ptr<Stmt> stmt = ParseStatement();

        if(!stmt)
            return nullptr;       

        shared_ptr<Type> stmtRetType = stmt->hasRet();
        if(stmtRetType && !retType)
            retType = stmtRetType;
        else if(retType && stmtRetType && !checkTypes(retType, stmtRetType))
            return LogStmtError("invalid returns types");
                    
        stmts.push_back(std::move(stmt));
    }
    prevTable();
    nextToken();

    return make_unique<ParenStmts>(std::move(stmts), retType);
}

static unique_ptr<Stmt> ParseRetStmt() {
    nextToken();

    unique_ptr<Expr> expr = ParseExpression();
    if(!expr)
        return nullptr;
    
    if(CurrTok.tok != TOKEN::SEMICOL)
        return LogStmtError("excepted ';'");
    nextToken();

    return make_unique<RetStmt>(std::move(expr));
}

static unique_ptr<Expr> ParseExpression() {
    unique_ptr<Expr> lhs = ParseBoolExpr();

    if(!lhs)
        return nullptr;

    if(CurrTok.tok != TOKEN::ASSIGN)
        return std::move(lhs);

    if(!lhs->lvalue())
        return LogExprError("excepted lvalue on left-side");

    unique_ptr<Lvalue> lexpr{ dynamic_cast<Lvalue*>(lhs.release()) };

    shared_ptr<Symbol> lsym = lexpr->getTable()->getSym(lexpr->getName());

    nextToken();
    unique_ptr<Expr> rhs = ParseExpression();
    
    if(!rhs)
        return nullptr;

    if(!checkTypes(lexpr->getType(), rhs->getType()))
        return LogExprError("invalid types");


    return make_unique<AssignExpr>(std::move(lexpr), std::move(rhs), lhs->getType());
}

static unique_ptr<Expr> ParseBoolExpr() {
    unique_ptr<Expr> lhs = ParseAddExpr();
    if(!lhs)
        return nullptr;

    while(true) {
        if((int)CurrTok.tok < 39 || (int)CurrTok.tok > 44)
            return std::move(lhs);

        string Op = CurrTok.word;
        nextToken();

        unique_ptr<Expr> rhs = ParseAddExpr();
        if(!rhs)
            return nullptr;
        
        if(!checkTypes(lhs->getType(), rhs->getType()) || lhs->getType()->get() == Type::Array)
            return LogExprError("invalid types");

        lhs = make_unique<BoolExpr>(Op, std::move(lhs), std::move(rhs), lhs->getType());
    }

    return LogExprError("invalid expr parsing (bool)");
}


static unique_ptr<Expr> ParseAddExpr() {
    unique_ptr<Expr> lhs = ParseTermExpr();

    if(!lhs)
        return nullptr;

    while(true) {
        if((int)CurrTok.tok < 33 || (int)CurrTok.tok > 34)
            return std::move(lhs);

        string Op = CurrTok.word;
        nextToken();

        unique_ptr<Expr> rhs = ParseTermExpr();

        if(!checkTypes(lhs->getType(), rhs->getType()) || lhs->getType()->get() == Type::Array)
            return LogExprError("invalid types");

        lhs = make_unique<AddExpr>(Op, std::move(lhs), std::move(rhs), lhs->getType());
    }

    return LogExprError("invalid expr parsing (add)");
}


static unique_ptr<Expr> ParseTermExpr() {
    unique_ptr<Expr> lhs = ParseFactor();
    if(!lhs)
        return nullptr;

    while(true) {
        if((int)CurrTok.tok < 35 || (int)CurrTok.tok > 36)
            return std::move(lhs);

        string Op = CurrTok.word;
        nextToken();

        unique_ptr<Expr> rhs = ParseFactor();
        if(!rhs)
            return nullptr;

        if(!checkTypes(lhs->getType(), rhs->getType()) || lhs->getType()->get() == Type::Array)
            return LogExprError("invalid types");

        lhs = make_unique<TermExpr>(Op, std::move(lhs), std::move(rhs), lhs->getType());
    }

    return LogExprError("invalid expr parsing (term)");
}
    
static unique_ptr<Expr> ParseFactor() {
    TOKEN PrevTok = CurrTok;

    nextToken();

    switch(PrevTok.tok) {
    case TOKEN::INTEGER: return make_unique<IntExpr>(PrevTok.ival);
    case TOKEN::TRUE: return make_unique<IntExpr>(1);
    case TOKEN::FALSE: return make_unique<IntExpr>(0);
    case TOKEN::LBRACE: {
        shared_ptr<Type> elemType = nullptr;
        vector<unique_ptr<Expr>> arr_elems;
        while(CurrTok.tok != TOKEN::RBRACE) {
            unique_ptr<Expr> elem = ParseExpression();
            if(!elem)
                return nullptr;
            
            if(!elemType)
                elemType = elem->getType();
            else if(!checkTypes(elemType, elem->getType()))
                return LogExprError("invalid elements in array");
            
            arr_elems.push_back(std::move(elem));
    
            if(CurrTok.tok != TOKEN::COMMA && CurrTok.tok != TOKEN::RBRACE)
                return LogExprError("excepted ']'");
            else if(CurrTok.tok == TOKEN::COMMA) {
                nextToken();
                if(CurrTok.tok == TOKEN::RBRACE)
                    return LogExprError("excepted array element");
            }
        }
        nextToken();
        return make_unique<ArrayExpr>(std::move(arr_elems), make_shared<ArrayType>(elemType));
    }

    case TOKEN::IDENTIFIER: {
        shared_ptr<Symbol> sym = CTable->getSym(PrevTok.word);
        if(!sym) return LogExprError("unknown identifier");
        if(CurrTok.tok == TOKEN::LBRACE) {
            if(sym->getSymType() != Symbol::FUNCTION)
                return LogExprError("invalid try call war");
                
            nextToken();

            size_t Idx = 0;
            const vector<shared_ptr<Type>>& args_types = sym->getArgs();

            vector<unique_ptr<Expr>> args;
            while(CurrTok.tok != TOKEN::RBRACE) {
                if(Idx == args_types.size())
                    return LogExprError("invalid number of args");

                unique_ptr<Expr> arg = ParseExpression();
                if(!arg)
                    return nullptr;

                if(!checkTypes(arg->getType(), args_types[Idx++]))
                    return LogExprError("invalid args types for call func");

                args.push_back(std::move(arg));

                if(CurrTok.tok != TOKEN::RBRACE && CurrTok.tok != TOKEN::COMMA)
                    return LogExprError("in call excepted ']'");
                else if(CurrTok.tok == TOKEN::COMMA) {
                    nextToken();
                    if(CurrTok.tok == TOKEN::RBRACE)
                        return LogExprError("excepted arg expr after comma");
                }
            }
            nextToken();
            
            if(Idx != args_types.size())
                return LogExprError("invalid number of args");

            return make_unique<CallExpr>(std::move(args), CTable, PrevTok.word, sym->getType());
        }
        else {
            if(sym->getSymType() != Symbol::WAR)
                return LogExprError("invalid func call");
           
            return make_unique<IDExpr>(PrevTok.word, CTable, sym->getType());
        }
    }
    case TOKEN::LBAR: {
        unique_ptr<Expr> expr = ParseExpression();
        if(!expr)
            return nullptr;

        if(CurrTok.tok != TOKEN::RBAR)
            return LogExprError("excepted ')' in expr");
        
        nextToken(); // eat RBAR
        return make_unique<ParenExpr>(std::move(expr));
    }
    // next cases
    default: {debug_tok(PrevTok) debug_tok(CurrTok) return LogExprError("unknown factor");}
    }

    return LogExprError("invalid expr parsing (factor)");
}

static shared_ptr<Type> ParseType() {
    shared_ptr<Type> result;
    Type::TypeWord type = getDType(CurrTok.tok);

    nextToken(); // eat typename
    switch(type) {
        case Type::Array: {
            if(CurrTok.tok != TOKEN::LS)
                return LogTypeError("excepted '<' for array subtype");      
      
            nextToken(); // eat <
            result = make_shared<ArrayType>(ParseType());

            if(CurrTok.tok != TOKEN::GT)
                return LogTypeError("excepted '>' for array subtype");

            nextToken(); // eat >
            break;
        }
        case Type::Int: result = make_shared<IntType>(); break;
        default: result = nullptr; break;
    }

    return result;
}

} // end parser namespace
