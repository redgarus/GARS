#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"

#include "type.hpp"

using llvm::AllocaInst, llvm::Function, llvm::Value;
using std::unordered_map, std::shared_ptr, std::make_shared, std::string, std::vector;

struct Symbol {
    enum SymType { WAR, TREN };
    AllocaInst *alloc = nullptr;
    Function *func = nullptr;
    vector<shared_ptr<ValueType>> args_types;
    shared_ptr<ValueType> type;
    SymType sym_type;
    string name;

    Symbol(const string& name, shared_ptr<ValueType> type, SymType sym_type)
        : name(name), type(type), sym_type(sym_type) {}
};

class Table {
    shared_ptr<Table> prev_tab;
    unordered_map<string, shared_ptr<Symbol>> symbols;
public:
    shared_ptr<Table> getPrev() { return prev_tab; }
};
