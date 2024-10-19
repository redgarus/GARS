#pragma once

#include "token.hpp"
#include "ast.hpp"
#include "table.hpp"

class Parser: public CompilerPass {
    vector<TOKEN> lex_tokens;
    TOKEN CurrTok; size_t i = 0;

    shared_ptr<Table> table = make_shared<Table>();
    
    TOKEN nextToken();
    
    void LogError(const string&);
    unique_ptr<Stmt> LogStmtError(const string&);
    unique_ptr<Expr> LogExprError(const string&);
    shared_ptr<ValueType> LogTypeError(const string&);
    
    unique_ptr<Stmt> ParseStatement(),
        ParseIfStmt(),
        ParseAliveStmt(),
        ParseParenStmts(),
        ParseTrenStmt(),
        ParseWarStmt(),
        ParseRetStmt(),
        ParseHighExpr();

    unique_ptr<Expr> ParseExpression(),
        ParseBoolExpr(),
        ParseAddExpr(),
        ParseTermExpr(),
        ParseFactor(),
        ParseIdentifier(),
        ParseInteger(),
        ParseTrueFalse(),
        ParseArray(),
        ParseParenExpr();

    shared_ptr<ValueType> ParseType(bool ptr_array=false);

public:
    unique_ptr<Input> ParseInput();

    void setTokens(vector<TOKEN> toks) { lex_tokens = std::move(toks); }
    void accept(shared_ptr<IVisitor> visitor) { visitor->visit(*this); }

    Parser() {}
};

