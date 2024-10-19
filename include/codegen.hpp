#pragma once

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"


#include "ast.hpp"
#include "visitor.hpp"

#include <unordered_map>

using std::unordered_map;

struct CodeVisitor: public ASTVisitor {
    Value *LogCodeError(const string&);
    static llvm::Type *convert(shared_ptr<ValueType>);
    static llvm::ArrayType *arr_convert(shared_ptr<ValueType>);

    Value *visit(WarStmt&);
    Function *visit(TrenStmt&);
    Value *visit(RetStmt&);
    Value *visit(IfStmt&);
    Value *visit(AliveStmt&);
    Value *visit(HighExpr&);
    Value *visit(ParenStmts&);

    Value *visit(AssignExpr&);
    Value *visit(BoolExpr&);
    Value *visit(AddExpr&);
    Value *visit(TermExpr&);
    Value *visit(IDExpr&);
    Value *visit(CallExpr&);
    Value *visit(IntExpr&);
    Value *visit(ArrayExpr&);
    Value *visit(ParenExpr&);
    Value *visit(IndexExpr&);

    Value *visit(Input&);

    unique_ptr<llvm::Module> getModule();
    
    void run();
};

struct AddrVisitor: public ASTVisitor {
    Value *visit(WarStmt&) { return nullptr; }
    Function *visit(TrenStmt&) { return nullptr; }
    Value *visit(RetStmt&) { return nullptr; }
    Value *visit(IfStmt&) { return nullptr; }
    Value *visit(AliveStmt&) { return nullptr; }
    Value *visit(HighExpr&) { return nullptr; }
    Value *visit(ParenStmts&) { return nullptr; }

    Value *visit(AssignExpr&);
    Value *visit(BoolExpr&) { return nullptr; }
    Value *visit(AddExpr&) { return nullptr; }
    Value *visit(TermExpr&) { return nullptr; }
    Value *visit(IDExpr&);
    Value *visit(CallExpr&) { return nullptr; }
    Value *visit(IntExpr&) { return nullptr; }
    Value *visit(ArrayExpr&) { return nullptr; }
    Value *visit(ParenExpr&);
    Value *visit(IndexExpr&);

    Value *visit(Input&) { return nullptr; }    
};


struct LLSym {
    string name;
    llvm::Type *type;
    llvm::AllocaInst *addr;

    LLSym(const string& name, llvm::Type *type, llvm::AllocaInst *alloc)
        : name(name), type(type), addr(alloc) {}
};

class Codegen: public CompilerPass {
public:
    void accept(shared_ptr<IVisitor> vis) { vis->visit(*this); }

    Codegen() {}
};
