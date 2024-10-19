#pragma once

#include "ast.hpp"
#include "type.hpp"

using std::unordered_map;

struct Symbol {
    virtual const string& getName() const = 0;
    virtual shared_ptr<ValueType> getType() const = 0;
    virtual const vector<pair<string, shared_ptr<ValueType>>>& getArgs() const = 0;
    
    virtual ~Symbol() = default;
};

struct ASTSym: public Symbol {
    string name;
    shared_ptr<ValueType> type;
    vector<pair<string, shared_ptr<ValueType>>> args{};

    const string& getName() const override { return name; }
    shared_ptr<ValueType> getType() const override { return type; }
    const vector<pair<string, shared_ptr<ValueType>>>& getArgs() const override { return args; }
    
    ASTSym(const string& name, shared_ptr<ValueType> type)
        : name(name), type(type) {}
    ASTSym(const string& name, shared_ptr<ValueType> type, vector<pair<string, shared_ptr<ValueType>>> args)
        : name(name), type(type), args(std::move(args)) {}
};

class Scope {
    unordered_map<string, shared_ptr<Symbol>> syms;
    string name;
public:
    const string& getName() const { return name; }
    
    shared_ptr<Symbol> find_symbol(const string& name) {
        return (syms.count(name)? syms[name] : nullptr);
    }

    void set_symbol(shared_ptr<Symbol> sym) {
        syms[sym->getName()] = sym;
    }

    Scope(const string& name) : name(name) {}
};

class Table {
    vector<shared_ptr<Scope>> stack;
public:
    shared_ptr<Scope> enter_scope(const string& name = "main") {
        shared_ptr<Scope> sym = make_shared<Scope>(name);
        stack.push_back(sym);
        return sym;
    }

    shared_ptr<Symbol> find_symbol(const string& name) {
        for(size_t i = 0, n = stack.size(); i < n; ++i)
            if(shared_ptr<Symbol> sym = stack[i]->find_symbol(name))
                return sym;

        return nullptr;
    }

    void add_symbol(shared_ptr<Symbol> sym) {
        stack.back()->set_symbol(sym);
    }

    shared_ptr<Scope> get_scope() {
        return stack.back();
    }
    
    void exit_scope() {
        stack.pop_back();
    }
};
