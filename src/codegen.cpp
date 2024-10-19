#include "../include/codegen.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>

using namespace llvm;
using namespace llvm::sys;

static unique_ptr<LLVMContext> LLCTX;
static unique_ptr<Module> TheModule;
static unique_ptr<IRBuilder<>> Builder;

unique_ptr<Module> CodeVisitor::getModule() {
    return std::move(TheModule);
}

using LLTableSymbol = unordered_map<string, shared_ptr<LLSym>>;

vector<LLTableSymbol> stack;

void enter_scope() {
    stack.push_back({});
}

void add_symbol(shared_ptr<LLSym> llsym) {    
    stack.back()[llsym->name] = llsym;
}

shared_ptr<LLSym> find_symbol(const string& name) {
    for(size_t i = stack.size() - 1; i >= 0; --i)
        if(stack[i].count(name))
            return stack[i][name];
    
    return nullptr;
}

void exit_scope() {
    stack.pop_back();
}


Type *CodeVisitor::convert(shared_ptr<ValueType> tval) {
    switch(tval->get()) {
    case ValueType::INT:
        return Type::getInt64Ty(*LLCTX);
    case ValueType::ARRAY: {
        if(tval->size() == 0)
            return PointerType::get(convert(tval->getSub()), 0);
        else
            return llvm::ArrayType::get(convert(tval->getSub()), tval->size());
    }
        
    default: return nullptr;
    }
}

llvm::ArrayType *CodeVisitor::arr_convert(shared_ptr<ValueType> arrType) {
    return llvm::ArrayType::get(convert(arrType->getSub()), arrType->size());
}

Value *CodeVisitor::LogCodeError(const string& msg) {
    std::cerr << "CodeGenError: " << msg << ".\n";
    return nullptr;
}

void CodeVisitor::run() {
    LLCTX = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("Module", *LLCTX);
    Builder = std::make_unique<IRBuilder<>>(*LLCTX);

    FunctionType *printf_ft = FunctionType::get(Type::getInt64Ty(*LLCTX), { PointerType::get(Type::getInt8Ty(*LLCTX), 0) }, true);
    Function *printf_f = Function::Create(printf_ft, Function::ExternalLinkage, "printf", TheModule.get());

    FunctionType *print_ft = FunctionType::get(Type::getInt64Ty(*LLCTX), { Type::getInt64Ty(*LLCTX) }, false);
    Function *print_f = Function::Create(print_ft, Function::ExternalLinkage, "print", TheModule.get());

    BasicBlock *print_mainbb = BasicBlock::Create(*LLCTX, "entry", print_f);

    Builder->SetInsertPoint(print_mainbb);

    Builder->CreateCall(printf_f, {
            Builder->CreateGlobalStringPtr("Output: %i\n", "out"),
            print_f->getArg(0)
        }, "calltmp");

    Builder->CreateRet(ConstantInt::get(*LLCTX, APInt(64, 0)));
    
    FunctionType *main_ft = FunctionType::get(Type::getInt64Ty(*LLCTX), false);
    Function *main_f = Function::Create(main_ft, Function::ExternalLinkage, "main", TheModule.get());

    BasicBlock *mainbb = BasicBlock::Create(*LLCTX, "entry", main_f);

    Builder->SetInsertPoint(mainbb);
    
    enter_scope();
}

Value *CodeVisitor::visit(Input& inp) {    
    for(size_t i = 0, s = inp.stmts.size(); i < s; ++i) {
        Value *stmtV = inp.stmts[i]->accept(*this);
    }

    Builder->CreateRet(ConstantInt::get(*LLCTX, APInt(64, 0)));
    
    return ConstantInt::get(*LLCTX, APInt(64, 0));
}

Value *CodeVisitor::visit(WarStmt& war) {
    Value *warValue = war.value->accept(*this);
    if(!warValue)
        return nullptr;
    
    Type *warType = convert(war.type);
    AllocaInst *warAddr = Builder->CreateAlloca(warType, nullptr, war.name);

    Builder->CreateStore(warValue, warAddr);

    add_symbol(make_shared<LLSym>(war.name, warType, warAddr));
    
    return ConstantInt::get(*LLCTX, APInt(64, 0));
}

Function *CodeVisitor::visit(TrenStmt& tren) {
    size_t n = tren.args.size();
    
    vector<Type *> Vargs;
    for(size_t i = 0; i < n; ++i)
        Vargs.push_back(convert(tren.args[i].second));

    Type *funcType = convert(tren.retType);
    FunctionType *ft = FunctionType::get(funcType, Vargs, false);
    Function *func = Function::Create(ft, Function::ExternalLinkage, tren.name, TheModule.get());
    
    enter_scope();

    BasicBlock *prevbb = Builder->GetInsertBlock();
    
    BasicBlock *entry = BasicBlock::Create(*LLCTX, "entry", func);

    Builder->SetInsertPoint(entry);
    
    size_t I = 0;
    for(auto& Arg: func->args()) {
        string argName = tren.args[I].first;
        Arg.setName(argName);

        AllocaInst *arg_addr = Builder->CreateAlloca(Arg.getType(), nullptr);

        Builder->CreateStore(&Arg, arg_addr);

        add_symbol(make_shared<LLSym>(argName, Vargs[I], arg_addr));
        
        ++I;
    }

    Value *BodyV = tren.func_body->accept(*this);
    if(!BodyV)
        return nullptr;

    exit_scope();
    
    verifyFunction(*func);

    Builder->SetInsertPoint(prevbb);
    
    return func;
}

Value *CodeVisitor::visit(RetStmt& ret) {
    Function *retFunc = Builder->GetInsertBlock()->getParent();

    Value *retExpr = ret.expr->accept(*this);
    if(!retExpr)
        return nullptr;

    if(retFunc->getFunctionType()->isPointerTy()) {
        AllocaInst *retAddr = Builder->CreateAlloca(retFunc->getFunctionType(), nullptr, "ret");
        Builder->CreateStore(retExpr, retAddr);

        retExpr = retAddr;
    }
    
    Builder->CreateRet(retExpr);
    
    return ConstantInt::get(*LLCTX, APInt(64, 0));
}

Value *CodeVisitor::visit(IfStmt& ifstmt) {
    Function *TheFunction = Builder->GetInsertBlock()->getParent();

    Value *CondV = ifstmt.Cond->accept(*this);
    if(!CondV)
        return nullptr;

    CondV = Builder->CreateICmpNE(CondV, ConstantInt::get(*LLCTX, APInt(64, 0)), "ifcond");

    BasicBlock *BodyBB = BasicBlock::Create(*LLCTX, "ifbody", TheFunction);
    BasicBlock *nextBB = BasicBlock::Create(*LLCTX, "next", TheFunction);

    Builder->CreateCondBr(CondV, BodyBB, nextBB);

    Builder->SetInsertPoint(BodyBB);
    
    Value *BodyV = ifstmt.Body->accept(*this);
    if(!BodyV)
        return nullptr;

    Builder->CreateBr(nextBB);

    Builder->SetInsertPoint(nextBB);

    return ConstantInt::get(*LLCTX, APInt(64, 0));
}


Value *CodeVisitor::visit(AliveStmt& alive) {
    Function *TheFunction = Builder->GetInsertBlock()->getParent();

    Value *CondV = alive.Cond->accept(*this);
    if(!CondV)
        return nullptr;

    BasicBlock *CondBB = BasicBlock::Create(*LLCTX, "alivecondblock", TheFunction);
    BasicBlock *BodyBB = BasicBlock::Create(*LLCTX, "alivebody", TheFunction);    
    BasicBlock *NextBB = BasicBlock::Create(*LLCTX, "next", TheFunction);

    Builder->CreateBr(CondBB);
    
    Builder->SetInsertPoint(CondBB);

    CondV = Builder->CreateICmpNE(CondV, ConstantInt::get(*LLCTX, APInt(64, 0)), "alivecond");

    Builder->CreateCondBr(CondV, BodyBB, NextBB);

    Builder->SetInsertPoint(BodyBB);

    Value *BodyV = alive.Body->accept(*this);
    if(!BodyV)
        return nullptr;

    Builder->CreateBr(CondBB);

    Builder->SetInsertPoint(NextBB);

    return ConstantInt::get(*LLCTX, APInt(64, 0));
}


Value *CodeVisitor::visit(HighExpr& hexpr) {
    Value *Vexpr = hexpr.expr->accept(*this);
    if(!Vexpr)
        return nullptr;

    return Vexpr;
}

Value *CodeVisitor::visit(ParenStmts& paren) {
    enter_scope();

    for(size_t i = 0, e = paren.stmts.size(); i < e; ++i) {
        Value *stmt = paren.stmts[i]->accept(*this);
        if(!stmt)
            return nullptr;
    }

    exit_scope();
    return ConstantInt::get(*LLCTX, APInt(64, 0));
}


Value *CodeVisitor::visit(AssignExpr& assign) {
    AddrVisitor *addr_vis = new AddrVisitor();
    
    Value *lhs = assign.LHS->accept(*addr_vis);
    if(!lhs)
        return nullptr;
    
    Value *rhs = assign.RHS->accept(*this);
    
    Builder->CreateStore(rhs, lhs);

    return rhs;
}

Value *CodeVisitor::visit(BoolExpr& boolexpr) {
    Value *lhs = boolexpr.LHS->accept(*this), *rhs = boolexpr.RHS->accept(*this);
    
    if(!lhs || !rhs)
        return nullptr;
    Value *val;
    switch(boolexpr.OP) {
    case TOKEN::LS: val = Builder->CreateICmpSLT(lhs, rhs, "booltmp"); break;
    case TOKEN::GT: val = Builder->CreateICmpSGT(lhs, rhs, "booltmp"); break;
    case TOKEN::LSEQ: val = Builder->CreateICmpSLE(lhs, rhs, "booltmp"); break;
    case TOKEN::GTEQ: val = Builder->CreateICmpSGE(lhs, rhs, "booltmp"); break;
    case TOKEN::EQ: val = Builder->CreateICmpEQ(lhs, rhs, "booltmp"); break;
    case TOKEN::NOEQ: val = Builder->CreateICmpNE(lhs, rhs, "booltmp"); break;
    default: return LogCodeError("undefined operator for bool");
    }

    val = Builder->CreateIntCast(val, Type::getInt64Ty(*LLCTX), true, "intcast");
    
    return val;
}


Value *CodeVisitor::visit(AddExpr& add) {
    Value *lhs = add.LHS->accept(*this), *rhs = add.RHS->accept(*this);
    
    if(!lhs || !rhs)
        return nullptr;

    switch(add.OP) {
    case TOKEN::PLUS: return Builder->CreateAdd(lhs, rhs, "addtmp");
    case TOKEN::MINUS: return Builder->CreateSub(lhs, rhs, "addtmp");
    default: return LogCodeError("undefined operator for bool");
    }

    return nullptr;
}

Value *CodeVisitor::visit(TermExpr& term) {
    Value *lhs = term.LHS->accept(*this), *rhs = term.RHS->accept(*this);
    
    if(!lhs || !rhs)
        return nullptr;

    switch(term.OP) {
    case TOKEN::MUL: return Builder->CreateMul(lhs, rhs, "addtmp");
    case TOKEN::DIV: return Builder->CreateSDiv(lhs, rhs, "addtmp");
    default: return LogCodeError("undefined operator for bool");
    }

    return nullptr;
}

Value *CodeVisitor::visit(IDExpr& idexp) {
    shared_ptr<LLSym> sym = find_symbol(idexp.name);
    if(!sym)
        return LogCodeError("not found this id");
    
    return Builder->CreateLoad(sym->addr->getAllocatedType(), sym->addr, "idexpr");
}

Value *CodeVisitor::visit(CallExpr& call) {
    Function *func = TheModule->getFunction(call.name);

    AddrVisitor *addr_vis = new AddrVisitor();

    size_t I = 0;
    std::vector<Value *> args;

    Value *argV;
    for(auto& arg: call.args) {
        if(func->getArg(I)->getType()->isPointerTy())
            argV = arg->accept(*addr_vis);
        else
            argV = arg->accept(*this);
        
        args.push_back(argV);
        
        ++I;
    }
    
    return Builder->CreateCall(func, args, "calltmp");
}

Value *CodeVisitor::visit(IntExpr& iexpr) {
    return ConstantInt::get(*LLCTX, APInt(64, iexpr.value));
}

Value *CodeVisitor::visit(ArrayExpr& array) {
    Type *array_type = convert(array.type);
    AllocaInst *arr_alloc = Builder->CreateAlloca(array_type, nullptr, "arrtemp");
    
    for(size_t i = 0, e = array.elements.size(); i < e; ++i) {
        Value *gep = Builder->CreateInBoundsGEP(array_type, arr_alloc, {
                ConstantInt::get(*LLCTX, APInt(64, 0)),
                ConstantInt::get(*LLCTX, APInt(64, i))
            });

        Builder->CreateStore(array.elements[i]->accept(*this), gep);
    }
    
    return Builder->CreateLoad(array_type, arr_alloc, "arrloadtemp");
}

Value *CodeVisitor::visit(ParenExpr& pexpr) {
    return pexpr.expr->accept(*this);
}

Value *CodeVisitor::visit(IndexExpr& indexp) {
    shared_ptr<LLSym> sym = find_symbol(indexp.name);

    Type *gep_type = sym->addr->getAllocatedType();// convert(indexp.);
    Value *gep_addr = sym->addr;
    if(sym->type->isPointerTy())
        gep_addr = Builder->CreateLoad(sym->type, sym->addr);
    
    std::vector<Value *> Ids{ ConstantInt::get(*LLCTX, APInt(64, 0)) };
    for(size_t i = 0, e = indexp.Idxs.size(); i < e; ++i)
        Ids.push_back(indexp.Idxs[i]->accept(*this));
    
    Value *gep = Builder->CreateInBoundsGEP(gep_type, gep_addr, Ids, "gep");

    return Builder->CreateLoad(convert(indexp.type), gep);
}

// AddrVisitor

Value *AddrVisitor::visit(IDExpr& expr) {
    shared_ptr<LLSym> sym = find_symbol(expr.name);
    
    if(expr.type->get() == ValueType::ARRAY)
        return Builder->CreateGEP(sym->type, sym->addr, { ConstantInt::get(*LLCTX, APInt(64, 0)), ConstantInt::get(*LLCTX, APInt(64, 0)) });
    
    return sym->addr;
}

Value *AddrVisitor::visit(IndexExpr& indexp) {
    shared_ptr<LLSym> sym = find_symbol(indexp.name);

    Type *gep_type = CodeVisitor::convert(indexp.type);// convert(indexp.);
    Value *gep_addr = sym->addr;
    
    std::vector<Value *> Ids{ ConstantInt::get(*LLCTX, APInt(64, 0)) };
    
    CodeVisitor *code_vis = new CodeVisitor();
    
    for(size_t i = 0, e = indexp.Idxs.size(); i < e; ++i)
        Ids.push_back(indexp.Idxs[i]->accept(*code_vis));

    delete code_vis;
    
    return Builder->CreateGEP(sym->type, gep_addr, Ids, "gep");
}



Value *AddrVisitor::visit(AssignExpr& assign) {
    Value *lhs = assign.LHS->accept(*this);
    if(!lhs)
        return nullptr;

    CodeVisitor *code_vis = new CodeVisitor();
    
    Value *rhs = assign.RHS->accept(*code_vis);
    
    Builder->CreateStore(rhs, lhs);

    delete code_vis;
    
    return lhs;
}


Value *AddrVisitor::visit(ParenExpr& pexpr) {
    return pexpr.expr->accept(*this);
}
