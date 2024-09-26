#include "parser.hpp"

namespace Parser {

#define debug_tok(toke) std::cout << "TOKEN: " << toke.tok << ". Line: " << toke.line << ". Word: " << toke.word << ".\n";
    
using lexer::getNextToken;
using std::cerr;

    
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

//* Parser Block
unique_ptr<Input> ParseInput() {
    nextToken();
    nextTable();

    vector<unique_ptr<Stmt>> stmts;

    while(CurrTok.tok != TOKEN::EOFILE) {
        unique_ptr<Stmt> stmt = ParseStatement();

        if(!stmt)
            return nullptr;

        stmts.push_back(std::move(stmt));

        if(CurrTok.tok != TOKEN::SEMICOL && CurrTok.tok != TOKEN::RBRA)
            return LogInputError("missing semicol ';'");
        
        nextToken();
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

    if(cond->getType() != Type::Int)
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
    
    if(cond->getType() != Type::Int)
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
    vector<unique_ptr<AssignExpr>> wars;

    nextToken();
    while(CurrTok.tok != TOKEN::SEMICOL) {
        if(CurrTok.tok != TOKEN::IDENTIFIER)
            return LogStmtError("invalid identifier for war");

        string warName = CurrTok.word;

        nextToken();
        if(CurrTok.tok != TOKEN::COL) 
            return LogStmtError("excepted ':'");

        nextToken();
        Type type = getDType(CurrTok.tok);
        if(type == Type::Null)
            return LogStmtError("undefined type");
        
        nextToken();
        if(CurrTok.tok != TOKEN::ASSIGN)
            return LogStmtError("excepted '='");
        
        nextToken();
        unique_ptr<Expr> valu = ParseExpression();

        if(!valu)
            return nullptr;
        
        if(valu->getType() != type)
            return LogStmtError("invalid war type");

        wars.push_back(make_unique<AssignExpr>(make_unique<IDExpr>(warName, CTable, type), std::move(valu), type));
        CTable->addSym(warName, make_shared<Symbol>(warName, type, Symbol::WAR));

        if(CurrTok.tok != TOKEN::SEMICOL && CurrTok.tok != TOKEN::COMMA)
            return LogStmtError("excepted ';'");
        else if(CurrTok.tok == TOKEN::COMMA)
            nextToken();
    }

    return make_unique<WarStmt>(std::move(wars), CTable);
}

static unique_ptr<Stmt> ParseTrenStmt() {
    nextToken();
    Type func_type = getDType(CurrTok.tok);
    if(func_type == Type::Null)
        return LogStmtError("invalid func type");
    
    nextToken();
    if(CurrTok.tok != TOKEN::IDENTIFIER)
        return LogStmtError("excepted identifier");
    
    string func_name = CurrTok.word;

    nextToken();
    if(CurrTok.tok != TOKEN::LBRACE)
        return LogStmtError("excepted '['");

    CTable->addSym(func_name, make_shared<Symbol>(func_name, func_type, Symbol::FUNCTION));
    
    shared_ptr<Table> func_tab = nextTable();
    nextToken();

    vector<string> args;
    while(CurrTok.tok != TOKEN::RBRACE) {
        Type type = getDType(CurrTok.tok);
        if(type == Type::Null)
            return LogStmtError("invalid type");
        
        nextToken();
        if(CurrTok.tok != TOKEN::IDENTIFIER)
            return LogStmtError("excepted arg identifier");
        
        string name = CurrTok.word;
    
        CTable->addSym(name, make_shared<Symbol>(name, type));
        args.push_back(name);

        nextToken();
        if(CurrTok.tok != TOKEN::RBRACE && CurrTok.tok != TOKEN::COMMA)
            return LogStmtError("excepted ']'");
        else if(CurrTok.tok == TOKEN::COMMA)
            nextToken();
    }

    nextToken();
    unique_ptr<Stmt> body = ParseStatement();

    if(!body)
        return nullptr;
    
    if(!body->hasRet())
        return LogStmtError("excepted return statement in function body");

    prevTable();

    return make_unique<TrenStmt>(func_name, std::move(body), std::move(args), func_tab, func_type);
}

static unique_ptr<Stmt> ParseHighExpr() {
    unique_ptr<Expr> expr = ParseExpression();
    if(!expr)
        return nullptr;
    
    return make_unique<HighExpr>(std::move(expr));
}

static unique_ptr<Stmt> ParseParenStmts() {
    vector<unique_ptr<Stmt>> stmts;
    bool HasRetVal = false;

    nextToken();
    while(CurrTok.tok != TOKEN::RBRA) {
        unique_ptr<Stmt> stmt = ParseStatement();

        if(!stmt)
            return nullptr;
        
        HasRetVal = HasRetVal || stmt->hasRet();
    
        stmts.push_back(std::move(stmt));

        nextToken();
    }

    return make_unique<ParenStmts>(std::move(stmts), HasRetVal);
}

static unique_ptr<Stmt> ParseRetStmt() {
    nextToken();

    unique_ptr<Expr> expr = ParseExpression();
    if(!expr)
        return nullptr;
    
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

    Type warType = lexpr->getType();
    if(rhs->getType() != warType)
        return LogExprError("invalid types");

    return make_unique<AssignExpr>(std::move(lexpr), std::move(rhs), warType);
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

        if(lhs->getType() != rhs->getType())
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

        if(lhs->getType() != rhs->getType())
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

        if(lhs->getType() != rhs->getType())
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
    case TOKEN::TRUE: return make_unique<TrueExpr>(1);
    case TOKEN::FALSE: return make_unique<TrueExpr>(0);
    case TOKEN::STRING: return make_unique<StrExpr>(PrevTok.word);
    case TOKEN::IDENTIFIER: {
        shared_ptr<Symbol> sym = CTable->getSym(PrevTok.word);
        if(!sym) return LogExprError("unknown identifier");
        if(CurrTok.tok == TOKEN::LBRACE) {
            if(sym->getSymType() != Symbol::FUNCTION)
                return LogExprError("invalid try call war");
                
            nextToken();
            
            vector<unique_ptr<Expr>> args;
            while(CurrTok.tok != TOKEN::RBRACE) {
                unique_ptr<Expr> arg = ParseExpression();
                if(!arg)
                    return nullptr;

                args.push_back(std::move(arg));

                if(CurrTok.tok != TOKEN::RBRACE && CurrTok.tok != TOKEN::COMMA)
                    return LogExprError("in call excepted ']'");
                else if(CurrTok.tok == TOKEN::COMMA)
                    nextToken();
            }
            nextToken();

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
    
} // end parser namespace
