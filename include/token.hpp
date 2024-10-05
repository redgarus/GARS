#pragma once

#include <iostream>
#include <string>

using std::string;
using std::ostream;

using ll = long long;

struct TOKEN {
    enum lexeme{
      
        // Liters
        INTEGER, TRUE, FALSE, STRING, REAL,

        // KeyWords
        IDENTIFIER, IF, ALIVE, WAR, YOU, WANT, 
        THIS, DO, NOTHING, BY, REDGAR, FIGHTCLUB, 
        TREN, RETURN, 

        // Types
        ARRAYTYPE, INTTYPE, BOOLTYPE, NONETYPE, REALTYPE,

        // Ops
        LBRA, RBRA, LBAR, RBAR, LBRACE, RBRACE,
        SEMICOL, COMMA, COL,
        PLUS, MINUS, 
        DIV, MUL,
        ASSIGN,
        NOT, LS, GT, EQ, NOEQ, GTEQ, LSEQ,

        // eof-tok and undefned-tok
        UNDEFINED, EOFILE, ERROR
    };

    ll ival = 0;
    string word;
    lexeme tok;
    int line;

    TOKEN(lexeme tok, const string& word, int line) : tok(tok), word(word), line(line) {};
    TOKEN(lexeme tok, ll ival, int line) : tok(tok), ival(ival), line(line) {};
    TOKEN(lexeme tok, int line) : tok(tok), line(line) {}
    TOKEN();
};

ostream& operator<<(std::ostream& out, TOKEN tok);
bool operator==(TOKEN tok, TOKEN::lexeme tok_lex);
