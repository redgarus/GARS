#include "../include/visitor.hpp"
#include "../include/codegen.hpp"
#include "../include/parser.hpp"
#include "../include/lexer.hpp"
#include "../include/table.hpp"
#include "../include/type.hpp"

#include <fstream>
#include <sstream>

using namespace llvm;
using namespace llvm::sys;

int GenerateObjFile(std::string Filename, unique_ptr<Module> TheModule) {
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


int main(int argc, char *argv[]) {
    std::ifstream file(argv[1]);
    std::stringstream ss;
    ss << file.rdbuf();

    shared_ptr<CompilerVisitor> comp_vis = make_unique<CompilerVisitor>();
    
    unique_ptr<Lexer> lexer = make_unique<Lexer>(ss.str(), ss.str().size());

    lexer->accept(comp_vis);

    /*
    for(auto token: comp_vis->tokens) {
        std::cout << token.tok << "\n";
    }
    */
    
    unique_ptr<Parser> parsec = make_unique<Parser>();

    parsec->accept(comp_vis);

    if(!comp_vis->AST)
        return 1;

    unique_ptr<Codegen> codegen = make_unique<Codegen>();

    codegen->accept(comp_vis);

    GenerateObjFile("redtest.o", std::move(comp_vis->mod));
    
    std::cout << "Compiling finished\n";
}
