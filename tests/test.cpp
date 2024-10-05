#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
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
#include "llvm/IR/GEPNoWrapFlags.h"

#include <bits/stdc++.h>
#include <llvm-18/llvm/ADT/APInt.h>

using namespace llvm;
using namespace llvm::sys;

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;

static void InitializeModule() {
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

int main() {
    InitializeModule();

    std::vector<Type *> printf_args{ PointerType::get(Type::getInt8Ty(*TheContext), 0) };
    FunctionType *printf_type = FunctionType::get(Type::getInt32Ty(*TheContext), printf_args, true);
    Function *printf_func = Function::Create(printf_type, Function::ExternalLinkage, "printf", TheModule.get());


    FunctionType *main_type = FunctionType::get(Type::getInt64Ty(*TheContext), false);
    Function *main_func = Function::Create(main_type, Function::ExternalLinkage, "main", TheModule.get());
    
    BasicBlock *entry = BasicBlock::Create(*TheContext, "entry", main_func);        

    StructType *myVec = StructType::create(*TheContext,  {
            Type::getInt8Ty(*TheContext)->getPointerTo(),
            Type::getInt32Ty(*TheContext),
            Type::getInt32Ty(*TheContext)
        }, "vector");
    
    
    Builder->SetInsertPoint(entry);

    AllocaInst *test_alloc = Builder->CreateAlloca(myVec, nullptr, "test_alloc");
    
    Builder->CreateRet(ConstantInt::get(*TheContext, APInt(64, 0, true)));
    
    TheModule->print(errs(), nullptr);
    
    auto Filename = "llvm_tesss.o";
    GenerateObjFile(Filename);

    return 0;
}
