#include "lexer/lexer.hpp"
#include "ast/ast.hpp"
#include "parser/parser.hpp"

#include <fstream>
#include <sstream>

using std::cout, std::cin;
using std::ifstream, std::stringstream, std::string;
using lexer::setCode;

int main(int argc, char* argv[]) {
    ifstream file(argv[1]);
    stringstream ss;
    ss << file.rdbuf();
    
    string text = ss.str();
    setCode(text, text.size());
    
    file.close();

    std::unique_ptr<AST::Input> inp = Parser::ParseInput();
    if(!inp)
        return 1;

    AST::MainPool();
    Value *inpV = inp->codegen();

    AST::CreateRet();

    auto filename = "comp_test.o";
    AST::GenerateObjFile(filename);

    std::unique_ptr<Module> mod = AST::getModule();

    mod->print(llvm::errs(), nullptr);

    return 0;
}
