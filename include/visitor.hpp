#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "token.hpp"

using std::vector, std::unique_ptr, std::shared_ptr;

class Lexer;
class Parser;
class Codegen;
class Node;

class IVisitor {
public:
    virtual void visit(Lexer&) = 0;
    virtual void visit(Parser&) = 0;
    virtual void visit(Codegen&) = 0;

    virtual ~IVisitor() = default;
};

class CompilerVisitor: public IVisitor {
public:
    vector<TOKEN> tokens;
    unique_ptr<Node> AST;
    string Code;

    void visit(Lexer&) override;
    void visit(Parser&) override;
    void visit(Codegen&) override;
};

class CompilerPass {
public:
    virtual void accept(shared_ptr<IVisitor>) = 0;

    virtual ~CompilerPass() = default;
};
