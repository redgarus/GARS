#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>

using std::shared_ptr, std::unordered_map, std::unordered_set, std::string;

struct ValueType {
    enum type {
        INT, ARRAY, NONETYPE
    };

    virtual shared_ptr<ValueType> getSub() const { return nullptr; }
    virtual type get() const { return NONETYPE; }
    virtual int size() const { return 0; }
    
    virtual ~ValueType() = default;
};

struct IntType: public ValueType {
    type get() const override { return INT; }
};

struct ArrayType: public ValueType {
    shared_ptr<ValueType> SubType;
    int arr_size;

    int size() const override { return arr_size; }
    type get() const override { return ARRAY; }
    
    shared_ptr<ValueType> getSub() const override { return SubType; }

    ArrayType(shared_ptr<ValueType> subt, int arr_size) : SubType(subt), arr_size(arr_size) {}
};

struct NoneType: public ValueType {};

shared_ptr<ValueType> maxType(shared_ptr<ValueType>, shared_ptr<ValueType>);
bool matchType(const std::string&, shared_ptr<ValueType>);

bool operator==(shared_ptr<ValueType>, ValueType::type);
bool operator!=(shared_ptr<ValueType>, ValueType::type);
bool operator==(shared_ptr<ValueType>, shared_ptr<ValueType>);
bool operator!=(shared_ptr<ValueType>, shared_ptr<ValueType>);
