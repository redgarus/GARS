#include "../include/lexer.hpp"

using std::cerr, std::stoll, std::unordered_map;

static unordered_map<string, TOKEN::lexeme> tokTable {
   // keywords
    {"if", TOKEN::IF}, {"alive", TOKEN::ALIVE}, {"by", TOKEN::BY},
    {"var", TOKEN::WAR}, {"you", TOKEN::YOU}, {"fn", TOKEN::TREN}, 
    {"REDGAR", TOKEN::REDGAR}, {"fightclub", TOKEN::FIGHTCLUB},
    {"want", TOKEN::WANT}, {"this", TOKEN::THIS}, {"do", TOKEN::DO},
    {"return", TOKEN::RETURN}, 
    
    // liters
    {"true", TOKEN::TRUE}, {"false", TOKEN::FALSE},

    // types
    {"array", TOKEN::ARRAYTYPE}, {"int", TOKEN::INTTYPE},

    // ops
    {"{", TOKEN::LBRA}, {"}", TOKEN::RBRA},
    {"(", TOKEN::LBAR}, {")", TOKEN::RBAR},
    {"[", TOKEN::LBRACE}, {"]", TOKEN::RBRACE},
    {":", TOKEN::COL}, {";", TOKEN::SEMICOL}, {",", TOKEN::COMMA}, {"|", TOKEN::ST},
    {"+", TOKEN::PLUS}, {"-", TOKEN::MINUS}, {"/", TOKEN::DIV}, {"*", TOKEN::MUL},
    {"!", TOKEN::NOT},  {"=", TOKEN::ASSIGN}, {"<", TOKEN::LS}, {">", TOKEN::GT},
    {"!=", TOKEN::NOEQ},  {"==", TOKEN::EQ}, {"<=", TOKEN::LSEQ}, {">=", TOKEN::GTEQ}
};

TOKEN Lexer::getNextToken() {
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

TOKEN Lexer::LexError(const string& msg) {
   cerr << "Syntax Error: " << msg << ". Line: " << line << ".\n";
   return TOKEN(TOKEN::ERROR, line);
}

ostream& operator<<(std::ostream& out, TOKEN tok) {
   return out << tok.word;
}

bool operator==(TOKEN tok, TOKEN::lexeme tok_lex) {
    return tok.tok == tok_lex;
}

bool operator!=(TOKEN tok, TOKEN::lexeme tok_lex) {
    return tok.tok != tok_lex;
}
