#include "../include/visitor.hpp"
#include "../include/codegen.hpp"
#include "../include/parser.hpp"
#include "../include/lexer.hpp"
#include "../include/token.hpp"

/* 
   тут должен быть accept для визитёра
   который будет посещать элементы классов Lexer, Parser, Codegen

   1. В результате прохода в Lexer в визитёре будет последовательность токенов

   2. В результате прохода в Parser AST

   3. В результате прохода в Codegen пока точно не знаю, но скорее всего llvm-ir

   Будет отдельный визитёр для этого и для AST элементов отдельный. (2 визитёра)
*/


void CompilerVisitor::visit(Lexer& lex) {
    TOKEN CurrTok = lex.getNextToken();
    
    while(!(CurrTok == TOKEN::EOFILE)) {
        tokens.push_back(CurrTok);
        CurrTok = lex.getNextToken();
    }
    tokens.push_back(CurrTok);
}

void CompilerVisitor::visit(Parser& parser) {
    parser.setTokens(std::move(tokens));
    AST = parser.ParseInput();
}

void CompilerVisitor::visit(Codegen& code) {
   CodeVisitor *visitor = new CodeVisitor();

   visitor->run();
    
   AST->accept(*visitor);
        
   mod = std::move(visitor->getModule());

   mod->print(llvm::errs(), nullptr);

   delete visitor;
}
