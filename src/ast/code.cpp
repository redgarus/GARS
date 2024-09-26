#include <iostream>
#include <memory>

#include "ast.hpp"

using ASType = AST::Type;
using LLType = llvm::Type;

using namespace AST;

using namespace llvm;
using namespace llvm::sys;

static unique_ptr<LLVMContext> TheContext;
static unique_ptr<Module> TheModule;
static unique_ptr<IRBuilder<>> Builder;

static void InitializeModule() {
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("my cool jit", *TheContext);
    Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

LLType *getLType(ASType type) {
    switch(type) {
    case ASType::Bool: return LLType::getInt1Ty(*TheContext);
    case ASType::Int: return LLType::getInt64Ty(*TheContext);
    case ASType::Null: return LLType::getInt64Ty(*TheContext);
    case ASType::String: return LLType::getInt8Ty(*TheContext);
    }
    return LLType::getInt64Ty(*TheContext);
}

Value *LogCompError(const string &message) {
    std::cerr << "Compile Error: " << message << ".\n";
    return nullptr;
}

Value *Input::codegen() {
    for(size_t i = 0, e = stmts.size(); i != e; ++i) {
        Value *stmt = stmts[i]->codegen();

        if(!stmt)
            return nullptr;
    }

    return ConstantInt::get(*TheContext, APInt(1, 0, true));
}

Value *AssignExpr::codegen() {
    shared_ptr<Symbol> sym = LHS->getTable()->getSym(LHS->getName());
    LLType *warType = getLType(sym->getType());
    
    AllocaInst *warAlloc = sym->getAlloc();

    if(!warAlloc)
        return LogCompError("undefined wariable " + string{LHS->getName()});
    Value *rhs = RHS->codegen();

    if(!rhs)
        return nullptr;

    Builder->CreateStore(rhs, warAlloc);

    return rhs;
}

Value *BoolExpr::codegen() {
    Value *lhs = LHS->codegen();
    Value *rhs = RHS->codegen();

    if(!lhs || !rhs)
        return nullptr;

    if(Vtype == ASType::String)
        return LogCompError("while not avaible logic ops on string type");

    Value *val;
    if(Op == "<")
        val = Builder->CreateICmpSLT(lhs, rhs, "booltmp");
    else if(Op == "<=")
        val = Builder->CreateICmpSLE(lhs, rhs, "booltmp");
    else if(Op == ">")
        val = Builder->CreateICmpSGT(lhs, rhs, "booltmp");
    else if(Op == ">=")
        val = Builder->CreateICmpSGE(lhs, rhs, "booltmp");
    else if(Op == "==")
        val = Builder->CreateICmpEQ(lhs, rhs, "booltmp");
    else if(Op == "!=")
        val = Builder->CreateICmpNE(lhs, rhs, "booltmp");
    else
        return LogCompError("invalid binary (bool) operator " + Op);

    return val;
}

Value *AddExpr::codegen() {
    Value *lhs = LHS->codegen();
    Value *rhs = RHS->codegen();

    if(!lhs || !rhs)
        return nullptr;
    
    Value *val;
    if(Op == "+") {
        
    }
    else if(Op == "-") {
        
    }

    return val;
}

