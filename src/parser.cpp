#include "../include/parser.hpp"

TOKEN Parser::nextToken() {
    return CurrTok = lex_tokens[++i];
}

void Parser::LogError(const string& msg) {
    std::cerr << "SyntaxError: " << msg << ". Line: " << CurrTok.line << ".\n";
}

unique_ptr<Stmt> Parser::LogStmtError(const string& msg) {
    LogError(msg);
    return nullptr;
}

unique_ptr<Expr> Parser::LogExprError(const string& msg) {
    LogError(msg);
    return nullptr;
}

shared_ptr<ValueType> Parser::LogTypeError(const string& msg) {
    LogError(msg);
    return nullptr;
}

unique_ptr<Input> Parser::ParseInput() {
    CurrTok = lex_tokens[i];

    table->enter_scope();

    vector<pair<string, shared_ptr<ValueType>>> print_args{ {"value", make_shared<IntType>() } };
    
    table->add_symbol(make_shared<ASTSym>("print", make_shared<IntType>(), std::move(print_args)));
    
    vector<unique_ptr<Stmt>> stmts;

    while(CurrTok != TOKEN::EOFILE) {
        unique_ptr<Stmt> stmt = ParseStatement();
        if(!stmt)
            return nullptr;
        
        stmts.push_back(std::move(stmt));
    }

    table->exit_scope();
    
    return make_unique<Input>(std::move(stmts));
}


unique_ptr<Stmt> Parser::ParseStatement() {
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

unique_ptr<Stmt> Parser::ParseIfStmt() {
    nextToken(); // eat if
    if(CurrTok != TOKEN::LBRACE)
        return LogStmtError("excepted '['");

    nextToken(); // eat [
    unique_ptr<Expr> cond = ParseExpression();
    if(!cond)
        return nullptr;

    if(cond->getType() != ValueType::INT)
        return LogStmtError("condition must be a integer");
    
    if(CurrTok != TOKEN::RBRACE)
        return LogStmtError("excepted ']'");

    nextToken();
    unique_ptr<Stmt> body = ParseStatement();
    if(!body)
        return nullptr;
    
    return make_unique<IfStmt>(std::move(cond), std::move(body));
}

unique_ptr<Stmt> Parser::ParseAliveStmt() {
    nextToken(); // eat alive

    if(CurrTok != TOKEN::BY)
        return LogStmtError("excepted 'by'");

    nextToken();
    if(CurrTok != TOKEN::LBRACE)
        return LogStmtError("excepted '['");

    nextToken();
    unique_ptr<Expr> cond = ParseExpression();
    if(!cond)
        return nullptr;

    if(cond->getType() != ValueType::INT)
        return LogStmtError("condition must be a integer");
    
    if(CurrTok != TOKEN::RBRACE)
        return LogStmtError("excepted ']'");

    nextToken();
    unique_ptr<Stmt> body = ParseStatement();
    if(!body)
        return nullptr;

    return make_unique<AliveStmt>(std::move(cond), std::move(body));
}

unique_ptr<Stmt> Parser::ParseWarStmt() {
    nextToken();

    if(CurrTok != TOKEN::IDENTIFIER)
        return LogStmtError("excepted identifier");

    string warName = CurrTok.word;

    nextToken();
    if(CurrTok != TOKEN::COL)
        return LogStmtError("excepted ';'");

    nextToken();
    shared_ptr<ValueType> warType = ParseType();
    if(!warType)
        return nullptr;
    
    if(CurrTok != TOKEN::ASSIGN)
        return LogStmtError("excepted '='");

    nextToken();
    unique_ptr<Expr> warValue = ParseExpression();
    if(!warValue)
        return nullptr;

    if(warType != warValue->getType())
        return LogStmtError("invalid war value");
    
    if(CurrTok != TOKEN::SEMICOL)
        return LogStmtError("excepted ';'");

    nextToken();

    table->add_symbol(make_shared<ASTSym>(warName, warType));
    
    return make_unique<WarStmt>(warName, std::move(warValue), warType);
}

unique_ptr<Stmt> Parser::ParseTrenStmt() {
    nextToken(); // eat tren

    if(CurrTok != TOKEN::IDENTIFIER)
        return LogStmtError("excepted identifier");

    string funcName = CurrTok.word;

    nextToken(); // eat identifier
    if(CurrTok != TOKEN::COL)
        return LogStmtError("excepted ':'");

    nextToken();
    
    shared_ptr<ValueType> funcType = ParseType(true);
    if(!funcType)
        return nullptr;
    
    if(CurrTok != TOKEN::LBRACE)
        return LogStmtError("excepted '['");

    shared_ptr<Scope> scope = table->get_scope();
    table->enter_scope(funcName);
    
    vector<pair<string, shared_ptr<ValueType>>> args;
    
    nextToken(); // eat [
    while(CurrTok != TOKEN::RBRACE) {
        shared_ptr<ValueType> arg_type = ParseType(true);
        if(!arg_type)
            return nullptr;

        if(CurrTok != TOKEN::IDENTIFIER)
            return LogStmtError("excepted identifier");

        string arg_name = CurrTok.word;

        args.push_back({arg_name, arg_type});

        table->add_symbol(make_shared<ASTSym>(arg_name, arg_type));
        
        nextToken();
        if(CurrTok != TOKEN::RBRACE && CurrTok != TOKEN::COMMA)
            return LogStmtError("excepted ']'");
        else if(CurrTok == TOKEN::COMMA) {
            nextToken(); // eat ,
            if(CurrTok == TOKEN::RBRACE)
                return LogStmtError("excepted argument");
        }
    }
    nextToken(); // eat ]

    scope->set_symbol(make_shared<ASTSym>(funcName, funcType, args));
    
    unique_ptr<Stmt> func_body = ParseStatement();
    if(!func_body)
        return nullptr;

    table->exit_scope();
    
    return make_unique<TrenStmt>(funcName, std::move(func_body), funcType, std::move(args));
}

unique_ptr<Stmt> Parser::ParseParenStmts() {
    nextToken(); // eat {
    
    vector<unique_ptr<Stmt>> stmts;

    table->enter_scope(table->get_scope()->getName());
    while(CurrTok != TOKEN::RBRA) {
        unique_ptr<Stmt> stmt = ParseStatement();
        if(!stmt)
            return nullptr;

        stmts.push_back(std::move(stmt));
    }
    table->exit_scope();
    
    nextToken();
    
    return make_unique<ParenStmts>(std::move(stmts));
}


unique_ptr<Stmt> Parser::ParseHighExpr() {
    unique_ptr<Expr> expr = ParseExpression();
    if(!expr)
        return nullptr;
    
    if(CurrTok != TOKEN::SEMICOL)
        return LogStmtError("excepted ';'");

    nextToken();
    
    return make_unique<HighExpr>(std::move(expr));
}

unique_ptr<Stmt> Parser::ParseRetStmt() {
    nextToken(); // eat return

    unique_ptr<Expr> retVal = ParseExpression();
    if(!retVal)
        return nullptr;

    shared_ptr<Scope> scope = table->get_scope();
    shared_ptr<Symbol> sym = table->find_symbol(scope->getName());

    if(sym->getType() != retVal->getType())
        return LogStmtError("invalid return type");
    
    if(CurrTok != TOKEN::SEMICOL)
        return LogStmtError("excepted ';'");

    nextToken();
    
    return make_unique<RetStmt>(std::move(retVal));
}

shared_ptr<ValueType> Parser::ParseType(bool fT) {
    switch(CurrTok.tok) {
    case TOKEN::ARRAYTYPE: {
        nextToken(); // eat array
        if(CurrTok != TOKEN::LS)
            return LogTypeError("excepted '<'");

        nextToken(); // eat <
        shared_ptr<ValueType> subType = ParseType();
        
        if(CurrTok != TOKEN::GT)
            return LogTypeError("excepted '>'");
        
        nextToken();
        if(CurrTok != TOKEN::LBRACE && fT)
            return make_shared<ArrayType>(subType, 0);
        else if(CurrTok != TOKEN::LBRACE)
            return LogTypeError("excepted '['");
        
        nextToken();
        if(CurrTok != TOKEN::INTEGER)
            return LogTypeError("excepted array size");

        int arr_size = CurrTok.ival;
        
        nextToken();
        if(CurrTok != TOKEN::RBRACE)
            return LogTypeError("excepted ']'");

        nextToken();

        return make_shared<ArrayType>(subType, arr_size);
    }
    case TOKEN::INTTYPE:
        nextToken();
        return make_shared<IntType>();
    default: return LogTypeError("excepted type");
    }
    
    return nullptr;
}

unique_ptr<Expr> Parser::ParseExpression() {
    unique_ptr<Expr> lhs = ParseBoolExpr();
    if(!lhs)
        return nullptr;

    if(CurrTok != TOKEN::ASSIGN)
        return std::move(lhs);
    
    nextToken(); // eat =
    unique_ptr<Expr> value = ParseExpression();
    if(!value)
        return nullptr;

    if(lhs->getType() != value->getType())
        return LogExprError("invalid types");
    
    return make_unique<AssignExpr>(std::move(lhs), std::move(value), lhs->getType());
}

unique_ptr<Expr> Parser::ParseBoolExpr() {
    unique_ptr<Expr> lhs = ParseAddExpr();
    if(!lhs)
        return nullptr;

    while(true) {
        if((int)CurrTok.tok < (int)TOKEN::LS || (int)CurrTok.tok > (int)TOKEN::LSEQ)
            return std::move(lhs);

        TOKEN::lexeme Op = CurrTok.tok;

        nextToken(); // eat Op

        unique_ptr<Expr> rhs = ParseAddExpr();
        if(!rhs)
            return nullptr;

        if(lhs->getType() != rhs->getType() ||
           !matchType("bool",maxType(lhs->getType(), rhs->getType())))
            return LogExprError("invalid types");
        
        lhs = make_unique<BoolExpr>(Op, std::move(lhs), std::move(rhs), lhs->getType());   
    }

    return LogExprError("whata fuck this error undefined");
}

unique_ptr<Expr> Parser::ParseAddExpr() {
    unique_ptr<Expr> lhs = ParseTermExpr();
    if(!lhs)
        return nullptr;

    while(true) {
        if((int)CurrTok.tok < (int)TOKEN::PLUS || (int)CurrTok.tok > (int)TOKEN::MINUS)
            return std::move(lhs);

        TOKEN::lexeme Op = CurrTok.tok;

        nextToken(); // eat Op

        unique_ptr<Expr> rhs = ParseTermExpr();
        if(!rhs)
            return nullptr;

        if(lhs->getType() != rhs->getType() ||
           !matchType("add", maxType(lhs->getType(), rhs->getType())))
            return LogExprError("invalid types");
        
        lhs = make_unique<AddExpr>(Op, std::move(lhs), std::move(rhs), lhs->getType());   
    }

    return LogExprError("whata fuck this error undefined");
}

unique_ptr<Expr> Parser::ParseTermExpr() {
    unique_ptr<Expr> lhs = ParseFactor();
    if(!lhs)
        return nullptr;

    while(true) {
        if((int)CurrTok.tok < (int)TOKEN::DIV || (int)CurrTok.tok > (int)TOKEN::MUL)
            return std::move(lhs);

        TOKEN::lexeme Op = CurrTok.tok;

        nextToken(); // eat Op

        unique_ptr<Expr> rhs = ParseFactor();
        if(!rhs)
            return nullptr;

        if(lhs->getType() != rhs->getType() ||
           !matchType("term", maxType(lhs->getType(), rhs->getType())))
            return LogExprError("invalid types");
        
        lhs = make_unique<TermExpr>(Op, std::move(lhs), std::move(rhs), lhs->getType());   
    }

    return LogExprError("whata fuck this error undefined");
}

unique_ptr<Expr> Parser::ParseFactor() {
    switch(CurrTok.tok) {
    case TOKEN::INTEGER:
        return ParseInteger();
    case TOKEN::TRUE: 
    case TOKEN::FALSE:
        return ParseTrueFalse();
    case TOKEN::LBRACE:
        return ParseArray();
    case TOKEN::IDENTIFIER:
        return ParseIdentifier();
    case TOKEN::LBAR:
        return ParseParenExpr();
        
    default: return LogExprError("unknown factor" + std::to_string((int)CurrTok.tok));        
    }
    
    return LogExprError("whata fuck this error undefined");
}

unique_ptr<Expr> Parser::ParseInteger() {
    ll value = CurrTok.ival;
    nextToken();
    return make_unique<IntExpr>(value);
}

unique_ptr<Expr> Parser::ParseTrueFalse() {
    ll value = (CurrTok == TOKEN::TRUE? 1:0);
    nextToken();
    return make_unique<IntExpr>(value);
}

unique_ptr<Expr> Parser::ParseArray() {
    nextToken();
    shared_ptr<ValueType> subType = make_shared<NoneType>();

    vector<unique_ptr<Expr>> elems;
    while(CurrTok != TOKEN::RBRACE) {
        unique_ptr<Expr> elem = ParseExpression();
        if(!elem)
            return nullptr;
        
        if(subType == ValueType::NONETYPE)
            subType = elem->getType();
        else if(elem->getType() != subType)
            return LogExprError("invalid array element type");
        
        elems.push_back(std::move(elem));
        
        if(CurrTok != TOKEN::RBRACE && CurrTok != TOKEN::COMMA)
            return LogExprError("excepted ]");
        else if(CurrTok == TOKEN::COMMA) {
            nextToken();
            if(CurrTok == TOKEN::RBRACE)
                return LogExprError("excepted array element");
        }
    }

    nextToken();

    return make_unique<ArrayExpr>(std::move(elems), make_shared<ArrayType>(subType, elems.size()));
}

unique_ptr<Expr> Parser::ParseIdentifier() {
    string IDName = CurrTok.word;

    nextToken();
    
    shared_ptr<Symbol> id_sym = table->find_symbol(IDName);
    if(!id_sym)
        return LogExprError("unknown identifier");

    if(CurrTok != TOKEN::LBAR && CurrTok != TOKEN::LBRACE) {
        if(id_sym->getArgs().size() > 0)
            return LogExprError("invalid call");

        return make_unique<IDExpr>(IDName, id_sym->getType());     
    }
    else if(CurrTok == TOKEN::LBRACE) {
        nextToken();

        shared_ptr<ValueType> Vtype = id_sym->getType();

        if(Vtype != ValueType::ARRAY)
            return LogExprError("identifier type must be array");
        
        vector<unique_ptr<Expr>> Idxs;
        while(CurrTok != TOKEN::RBRACE) {
            unique_ptr<Expr> index = ParseExpression();
            if(!index)
                return nullptr;

            if(index->getType() != ValueType::INT)
                return LogExprError("index must be integer");
            
            Idxs.push_back(std::move(index));
            
            if(CurrTok != TOKEN::COMMA && CurrTok != TOKEN::RBRACE)
                return LogExprError("excepted ']'");
            else if(CurrTok == TOKEN::COMMA) {
                nextToken();
                if(CurrTok == TOKEN::RBRACE)
                    return LogExprError("excepted index");
            }
            Vtype = Vtype->getSub();
        }
        nextToken();
        
        return make_unique<IndexExpr>(IDName, std::move(Idxs), Vtype);
    }    
    
    size_t I = 0;
    vector<unique_ptr<Expr>> args;
    
    nextToken(); // eat (
    while(CurrTok != TOKEN::RBAR) {
        if(I == id_sym->getArgs().size())
            return LogExprError("invalid number of args");
        
        unique_ptr<Expr> arg = ParseExpression();
        if(!arg)
            return nullptr;

        if(arg->getType() != id_sym->getArgs()[I++].second)
            return LogExprError("invalid types");
        
        args.push_back(std::move(arg));

        if(CurrTok != TOKEN::RBAR && CurrTok != TOKEN::COMMA)
            return LogExprError("excepted ')'");
        else if(CurrTok == TOKEN::COMMA) {
            nextToken();
            if(CurrTok == TOKEN::RBAR)
                return LogExprError("excepted argument expression after comma");
        }
    }
    
    nextToken(); // eat ]
    return make_unique<CallExpr>(IDName, std::move(args), id_sym->getType());
}

unique_ptr<Expr> Parser::ParseParenExpr() {
    nextToken(); // eat (
    unique_ptr<Expr> expr = ParseExpression();
    if(!expr)
        return nullptr;

    if(CurrTok != TOKEN::RBAR)
        return LogExprError("excepted )");
    
    nextToken();
    
    return make_unique<ParenExpr>(std::move(expr));
}
