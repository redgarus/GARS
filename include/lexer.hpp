#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>

#include "visitor.hpp"

using std::ostream;

class Lexer: public CompilerPass {
    size_t tsize, i = 0;
    int line = 1;
    string text;

    TOKEN LexError(const string&);
public:
    TOKEN getNextToken();

    void accept(shared_ptr<IVisitor> visitor) override { visitor->visit(*this); }

    Lexer(const string& text, size_t s) : text(text), tsize(s) {}
};
