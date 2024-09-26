#include "lexer.hpp"

#include <unordered_map>

namespace lexer {

using std::cerr, std::to_string, std::stoll, std::unordered_map;

static unordered_map<string, TOKEN::lexeme> tokTable {
    // keywords
    {"if", TOKEN::IF}, {"alive", TOKEN::ALIVE}, {"by", TOKEN::BY},
    {"war", TOKEN::WAR}, {"you", TOKEN::YOU}, {"tren", TOKEN::TREN}, 
    {"REDGAR", TOKEN::REDGAR}, {"fightclub", TOKEN::FIGHTCLUB},
    {"want", TOKEN::WANT}, {"this", TOKEN::THIS}, {"do", TOKEN::DO},
    {"return", TOKEN::RETURN}, 
    
    // liters
    {"true", TOKEN::TRUE}, {"false", TOKEN::FALSE},

    // types
    {"str", TOKEN::STRINGTYPE}, {"bool", TOKEN::BOOLTYPE}, {"int", TOKEN::INTTYPE},

    // ops
    {"{", TOKEN::LBRA}, {"}", TOKEN::RBRA},
    {"(", TOKEN::LBAR}, {")", TOKEN::RBAR},
    {"[", TOKEN::LBRACE}, {"]", TOKEN::RBRACE},
    {":", TOKEN::COL}, {";", TOKEN::SEMICOL}, {",", TOKEN::COMMA},
    {"+", TOKEN::PLUS}, {"-", TOKEN::MINUS}, {"/", TOKEN::DIV}, {"*", TOKEN::MUL},
    {"!", TOKEN::NOT},  {"=", TOKEN::ASSIGN}, {"<", TOKEN::LS}, {">", TOKEN::GT},
    {"!=", TOKEN::NOEQ},  {"==", TOKEN::EQ}, {"<=", TOKEN::LSEQ}, {">=", TOKEN::GTEQ}
};

TOKEN::lexeme getToken(const string& str_tok) {
    if(tokTable.count(str_tok))
        return tokTable[str_tok];
    return TOKEN::UNDEFINED;
}

static size_t tsize, i = 0;
static int line = 1;
static string text;

void setCode(const string& code, size_t s) {
    text = code; tsize = s;
}

TOKEN::TOKEN(lexeme tok, const string& word, int line)
    : tok(tok), word(word), line(line) {}
TOKEN::TOKEN(lexeme tok, ll val, int line)
    : tok(tok), ival(val), line(line) {}
TOKEN::TOKEN(lexeme tok, int line)
    : tok(tok), line(line) {}
TOKEN::TOKEN() {}

TOKEN getNextToken() {
    string word;

    if(i == tsize)
        return TOKEN(TOKEN::EOFILE, line);
    else if(isalpha(text[i])) {
        for(; isalnum(text[i]) && i < tsize; ++i)
            word += text[i];
        
        if(tokTable.count(word))
            return TOKEN(tokTable[word], line);

        return TOKEN(TOKEN::IDENTIFIER, word, line);
    }
    else if(isdigit(text[i])) {
        for(; isdigit(text[i]); ++i) {
            word += text[i];
        }

        return TOKEN(TOKEN::INTEGER, stoll(word), line);
    }
    else if(text[i] == '"') {
        for(++i; text[i] != '"' && i < tsize; ++i)
            word += text[i];
        
        if(i == tsize && text[i] != '"')
            return LexError("excepted '" + string{'"'} + "'");
        else if(i != tsize) ++i;
        
        return TOKEN(TOKEN::STRING, word, line);
    }
    else if(isspace(text[i])) {
        if(text[i++] == '\n') ++line;
        return getNextToken();
    }
    else if(ispunct(text[i])) {
        word += text[i++];
        if(!tokTable.count(word))
            return LexError("unknown punct");
        else if((int)tokTable[word] < (int)TOKEN::ASSIGN || (int)tokTable[word] > (int)TOKEN::LSEQ)
            return TOKEN(tokTable[word], line);
        
        if(i < tsize && text[i] == '=')
            word += text[i++];

        return TOKEN(tokTable[word], line);
    }

    return LexError("unknown char");
}

TOKEN LexError(const string& msg) {
    cerr << "Syntax Error: " << msg << ". Line: " << line << ".\n";
    return TOKEN(TOKEN::ERROR, line);
}

ostream& operator<<(ostream& out, TOKEN& tok) {
    out << "Lexeme: " << tok.tok;
    if(tok.word != "")
        out << " Value: " << tok.word << '.';
    else if(tok.ival)
        out << " Value: " << tok.ival << '.';
    
    return out << " Line: " << tok.line << '\n';
}

std::ostream& operator << (std::ostream& out, TOKEN::lexeme t) {
    switch(t) {
        case TOKEN::IF: return (out << "IF");
        case TOKEN::YOU: return (out << "YOU");
        case TOKEN::WANT: return (out << "WANT");
        case TOKEN::THIS: return (out << "THIS");
        case TOKEN::TREN: return (out << "TREN");
        case TOKEN::RETURN: return (out << "RETURN");
        case TOKEN::DO: return (out << "DO");
        case TOKEN::BY: return (out << "BY");
        case TOKEN::IDENTIFIER: return (out << "ID");
        case TOKEN::WAR: return (out << "WAR");
        case TOKEN::ALIVE: return (out << "ALIVE");
        case TOKEN::INTEGER: return (out << "INTEGER");
        case TOKEN::STRING: return (out << "STRING");
        case TOKEN::BOOLTYPE: return (out << "BOOLTYPE");
        case TOKEN::REALTYPE: return (out << "REALTYPE");
        case TOKEN::STRINGTYPE: return (out << "STRINGTYPE");
        case TOKEN::INTTYPE: return (out << "INTTYPE");

        case TOKEN::COMMA: return (out << "COMMA");
        case TOKEN::LBRA: return (out << "LBRA");
        case TOKEN::RBRA: return (out << "RBRA");
        case TOKEN::LBAR: return (out << "LBAR");
        case TOKEN::RBAR: return (out << "RBAR");
        case TOKEN::LS: return (out << "LESSER");
        case TOKEN::GT: return (out << "GREATER");
        case TOKEN::LBRACE: return (out << "LBRACE");
        case TOKEN::RBRACE: return (out << "RBRACE");
        case TOKEN::PLUS: return (out << "PLUS");
        case TOKEN::MINUS: return (out << "MINUS");
        case TOKEN::DIV: return (out << "DIV");
        case TOKEN::MUL: return (out << "MUL");
        case TOKEN::NOT: return (out << "NOT");
        case TOKEN::EQ: return (out << "EQUAL");
        case TOKEN::NOEQ: return (out << "NON EQUAL");
        case TOKEN::LSEQ: return (out << "LESS OR EQUAL");
        case TOKEN::GTEQ: return (out << "GREAT OR EQUAL");
        case TOKEN::ASSIGN: return (out << "ASSIGN");
        case TOKEN::SEMICOL: return (out << "SEMICOL");
        case TOKEN::COL: return (out << "COL");

        case TOKEN::EOFILE: return (out << "EOF");

        default: return (out << "UNDEFINED");
    }
}

} // lexer namespace ends
