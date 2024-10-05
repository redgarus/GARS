#pragma once

#include <memory>

using std::shared_ptr;

class ValueType {
public:
    enum type {
        INT, ARRAY, NONETYPE
    };

    virtual type get() const = 0;
    virtual shared_ptr<ValueType> getSub() const = 0;
    virtual ~ValueType() = default;
};

class IntType: public ValueType {
public:
    type get() const override { return INT; }
    shared_ptr<ValueType> getSub() const override { return nullptr; }
};

class ArrayType: public ValueType {
    shared_ptr<ValueType> SubType;
public:
    type get() const override { return ARRAY; }
    shared_ptr<ValueType> getSub() const override { return SubType; }
};

class NoneType: public ValueType {
public:
    type get() const override { return NONETYPE; }
    shared_ptr<ValueType> getSub() const override { return nullptr; }
};
