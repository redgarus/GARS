#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>

using std::string, std::ostream;
using ld = long double;
using ll = long long;

namespace lexer {

struct TOKEN {
    enum lexeme{
      
        // Liters
        INTEGER, TRUE, FALSE, STRING, REAL,

        // KeyWords
        IDENTIFIER, IF, ALIVE, WAR, YOU, WANT, 
        THIS, DO, NOTHING, BY, REDGAR, FIGHTCLUB, 
        TREN, RETURN, 

        // Types
        STRINGTYPE, INTTYPE, BOOLTYPE, NONETYPE, REALTYPE,

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

    TOKEN(lexeme, const string&, int);
    TOKEN(lexeme, ll, int);
    TOKEN(lexeme, int);
    TOKEN();
};

TOKEN getNextToken();
TOKEN::lexeme getToken(const string&);
TOKEN LexError(const string&);
void setCode(const string&, size_t);
ostream& operator<<(ostream&, TOKEN&);
ostream& operator<<(ostream&, TOKEN::lexeme);

}
#endif
