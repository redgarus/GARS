#include <iostream>
#include <llvm-18/llvm/ADT/APInt.h>
#include <llvm-18/llvm/IR/BasicBlock.h>
#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm-18/llvm/IR/Function.h>
#include <memory>

#include "ast.hpp"

using ASType = AST::Type;
using LLType = llvm::Type;

using std::make_shared;

using namespace llvm;
using namespace llvm::sys;

namespace AST {

unique_ptr<LLVMContext> TheContext;
unique_ptr<Module> TheModule;
unique_ptr<IRBuilder<>> Builder;

void InitializeModule() {
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("my cool jit", *TheContext);
    Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

int GenerateObjFile(std::string Filename) {
    // * GENERATE OBJ FILE
    // Initialize the target registry etc.
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto TargetTriple = sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto TheTargetMachine = Target->createTargetMachine(
        TargetTriple, CPU, Features, opt, Reloc::PIC_);

    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    legacy::PassManager pass;
    auto FileType = CodeGenFileType::ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    pass.run(*TheModule);
    dest.flush();

    outs() << "Wrote " << Filename << "\n";

    return 0;
}

void CreateRet() {
    Builder->CreateRet(ConstantInt::get(*TheContext, APInt(64, 0)));
}

void MainPool() {
    InitializeModule();

    // printf declaration
    FunctionType *printFT = FunctionType::get(LLType::getInt64Ty(*TheContext), { LLType::getInt8Ty(*TheContext) }, true);
    Function *printF = Function::Create(printFT, Function::ExternalLinkage, "printf", TheModule.get());
    
    std::vector<llvm::Type *> Args(1, llvm::Type::getInt64Ty(*TheContext));
    llvm::FunctionType *printeT = llvm::FunctionType::get(llvm::Type::getInt64Ty(*TheContext), Args, false);
    Function *printe = Function::Create(printeT, Function::ExternalLinkage, "printer", TheModule.get());    

    print_sym->setFunction(printe);

    for(auto &Arg: printe->args())
        Arg.setName("input");

    BasicBlock *mainBB = BasicBlock::Create(*TheContext, "mainblock", printe);

    Builder->SetInsertPoint(mainBB);

    Value *pres = Builder->CreateGlobalStringPtr("out: %i\n", "pernosik");

    std::vector<Value *> args{ pres };

    for(auto &Arg: printe->args()) {
        args.push_back(&Arg);
    }

    Builder->CreateCall(printF, args, "calltmp");

    Builder->CreateRet(ConstantInt::get(*TheContext, APInt(64, 0)));

    // main definition
    FunctionType *FT = FunctionType::get(LLType::getInt64Ty(*TheContext), {});
    Function *F = Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    BasicBlock *entryBB = BasicBlock::Create(*TheContext, "entry", F);

    Builder->SetInsertPoint(entryBB);
}

unique_ptr<Module> getModule() {
    return std::move(TheModule);
}

// getLType
static LLType *getLType(shared_ptr<ASType> type) {
    switch(type->get()) {
    case ASType::Int: return LLType::getInt64Ty(*TheContext);
    case ASType::Array: return getLType(type->getElementType())->getPointerTo();
    default: return nullptr;
    }
}

// Loggging
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
    return ConstantInt::get(*TheContext, APInt(64, 0));
}

Value *WarStmt::codegen() {
    for(size_t i = 0, e = wars.size(); i != e; ++i) {
        string warName = wars[i].first;
        unique_ptr<Expr> rhs = std::move(wars[i].second); 
        shared_ptr<Symbol> sym = CurrTable->getSym(warName);
        LLType *LexprType = getLType(rhs->getType());

        Value *number_of_elems = (rhs->size()? ConstantInt::get(*TheContext, APInt(64, rhs->size())) : nullptr);

        AllocaInst *alloc = Builder->CreateAlloca(LexprType, number_of_elems, warName);

        Value *Vrhs = rhs->codegen();
        if(!Vrhs)
            return nullptr;

        Builder->CreateStore(Vrhs, alloc);

        sym->setAlloc(alloc);
    }
    
    return ConstantInt::get(*TheContext, APInt(64, 0));
}

Function *TrenStmt::codegen() {
    shared_ptr<Symbol> func_sym = CurrTable->getSym(name);
    const std::vector<shared_ptr<ASType>>& ArgsTypes = func_sym->getArgs();

    std::vector<LLType *> VArgsTypes(ArgsTypes.size());

    for(size_t i = 0, e = ArgsTypes.size(); i != e; ++i) {
        VArgsTypes[i] = getLType(ArgsTypes[i]);
    }

    FunctionType *FT = FunctionType::get(getLType(Vtype), VArgsTypes, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());

    func_sym->setFunction(F);

    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    size_t Idx = 0;
    for(auto &Arg: F->args()) {
        Arg.setName(args[Idx]);
        
        AllocaInst *alloc = Builder->CreateAlloca(VArgsTypes[Idx], nullptr, args[Idx]);

        Builder->CreateStore(&Arg, alloc);
        
        shared_ptr<Symbol> arg_sym = CurrTable->getSym(args[Idx]);

        arg_sym->setAlloc(alloc);

        ++Idx;
    }

    Value *BodyV = body->codegen();
    if(!BodyV)
        return nullptr;
    
    verifyFunction(*F);

    return F;
}

Value *RetStmt::codegen() {
    Value *retExpr = expr->codegen();
    if(!retExpr)
        return nullptr;

    Builder->CreateRet(retExpr);

    return ConstantInt::get(*TheContext, APInt(64, 0));
}

Value *ParenStmts::codegen() {
    for(size_t i = 0, e = stmts.size(); i != e; ++i) {
        Value *stmtV = stmts[i]->codegen();
        if(!stmtV)
            return nullptr;
    }

    return ConstantInt::get(*TheContext, APInt(64, 0));
}

Value *IfStmt::codegen() {
    Function *TheFunction = Builder->GetInsertBlock()->getParent();

    Value *CondV = Cond->codegen();
    if(!CondV)
        return nullptr;

    CondV = Builder->CreateICmpNE(CondV, ConstantInt::get(*TheContext, APInt(64, 0)), "ifcond");

    BasicBlock *BodyBB = BasicBlock::Create(*TheContext, "ifbody", TheFunction);
    BasicBlock *nextBB = BasicBlock::Create(*TheContext, "next", TheFunction);

    Builder->CreateCondBr(CondV, BodyBB, nextBB);

    Builder->SetInsertPoint(BodyBB);
    
    Value *BodyV = Body->codegen();
    if(!BodyV)
        return nullptr;

    Builder->CreateBr(nextBB);

    Builder->SetInsertPoint(nextBB);

    return ConstantInt::get(*TheContext, APInt(64, 0));
}

Value *AliveStmt::codegen() {
    Function *TheFunction = Builder->GetInsertBlock()->getParent();

    Value *CondV = Cond->codegen();
    if(!CondV)
        return nullptr;

    BasicBlock *CondBB = BasicBlock::Create(*TheContext, "alivecondblock", TheFunction);
    BasicBlock *BodyBB = BasicBlock::Create(*TheContext, "alivebody", TheFunction);    
    BasicBlock *NextBB = BasicBlock::Create(*TheContext, "next", TheFunction);

    Builder->SetInsertPoint(CondBB);

    CondV = Builder->CreateICmpNE(CondV, ConstantInt::get(*TheContext, APInt(64, 0)), "alivecond");

    Builder->CreateCondBr(CondV, BodyBB, NextBB);

    Builder->SetInsertPoint(BodyBB);

    Value *BodyV = Body->codegen();
    if(!BodyV)
        return nullptr;

    Builder->CreateBr(CondBB);

    Builder->SetInsertPoint(NextBB);

    return ConstantInt::get(*TheContext, APInt(64, 0));
}

Value *HighExpr::codegen() {
    Value *ExprV = expr->codegen();
    if(!ExprV)
        return nullptr;

    return ExprV;
}

Value *AssignExpr::codegen() {
    Value *lhs = LHS->codegen(), *rhs = RHS->codegen();

    if(!lhs || !rhs)
        return nullptr;
    
    shared_ptr<Symbol> war_sym = LHS->getTable()->getSym(LHS->getName());
    
    Builder->CreateStore(rhs, war_sym->getAlloc());

    return rhs;
}

Value *BoolExpr::codegen() {
    Value *lhs = LHS->codegen(), *rhs = RHS->codegen();

    if(!lhs || !rhs)
        return nullptr;

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

    val = Builder->CreateIntCast(val, LLType::getInt64Ty(*TheContext), false, "restmp");

    return val;
}

Value *AddExpr::codegen() {
    Value *lhs = LHS->codegen(), *rhs = RHS->codegen();

    if(!lhs || !rhs)
        return nullptr;

    if(Op == "+")
        return Builder->CreateAdd(lhs, rhs, "inttmp");
    else if(Op == "-")
        return Builder->CreateSub(lhs, rhs, "inttmp");
    else
        return LogCompError("invalid binary (int) operator " + Op);
}

Value *TermExpr::codegen() {
    Value *lhs = LHS->codegen(), *rhs = RHS->codegen();

    if(!lhs || !rhs)
        return nullptr;

    if(Op == "*")
        return Builder->CreateMul(lhs, rhs, "termtmp");
    else if(Op == "/") {
        return Builder->CreateSDiv(lhs, rhs, "termtmp");;
    } else
        return LogCompError("invalid binary (term) operator " + Op);
}

Value *IntExpr::codegen() {
    return ConstantInt::get(*TheContext, APInt(64, value));
}

Value *ArrayExpr::codegen() {
    AllocaInst *alloc = Builder->CreateAlloca(getLType(Vtype->getElementType()), ConstantInt::get(*TheContext, APInt(64, value.size())), "array");
    
    for(size_t i = 0, e = value.size(); i != e; ++i) {
        Value *elemV = value[i]->codegen();
        if(!elemV)
            return nullptr;

        std::vector<Value *> Ids{ ConstantInt::get(*TheContext, APInt(64, i)) };
        Value *point = Builder->CreateGEP(getLType(Vtype->getElementType()), alloc, Ids, "index_query");

        Builder->CreateStore(elemV, point);
    }
    
    return alloc;
}

Value *IDExpr::codegen() {
    shared_ptr<Symbol> war_sym = CurrTable->getSym(name);

    Value *V = Builder->CreateLoad(getLType(Vtype), war_sym->getAlloc(), "idexpr");

   return V;
}

Value *CallExpr::codegen() {
    shared_ptr<Symbol> func_sym = CurrTable->getSym(name);

    std::vector<Value *> ArgsV;
    for(size_t i = 0, e = args.size(); i != e; ++i) {
        Value *ArgV = args[i]->codegen();
        if(!ArgV)
            return nullptr;

        ArgsV.push_back(ArgV);
    }

    return Builder->CreateCall(func_sym->getFunction(), ArgsV, "calltmp");
}

Value *ParenExpr::codegen() {
    Value *ExprV = expr->codegen();
    if(!ExprV)
        return nullptr;

    return ExprV;
}

}
