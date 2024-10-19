#include "../include/type.hpp"


unordered_map<string, unordered_set<ValueType::type>> typeTable{
    {"bool", {
            ValueType::INT
        }},
    {"add", {
            ValueType::INT
        }},
    {"term", {
            ValueType::INT
        }}
};

shared_ptr<ValueType> maxType(shared_ptr<ValueType> t1, shared_ptr<ValueType> t2) {
    if(t1->get() == ValueType::INT &&
       t2->get() == ValueType::INT)
        return t1;
    else if(t1->get() == ValueType::ARRAY &&
            t2->get() == ValueType::ARRAY)
        return t1;

    return nullptr;
}

bool matchType(const string& name, shared_ptr<ValueType> type) {
    return typeTable[name].count(type->get());
}


bool operator==(shared_ptr<ValueType> t1, ValueType::type t2)
{
    return t1->get() == t2;
}


bool operator!=(shared_ptr<ValueType> t1, ValueType::type t2)
{
    return !(t1 == t2);
}

bool operator==(shared_ptr<ValueType> t1, shared_ptr<ValueType> t2)
{
    if(t1->get() == ValueType::INT &&
       t2->get() == ValueType::INT)
        return true;
    else if(t1->get() == ValueType::ARRAY &&
            t2->get() == ValueType::ARRAY) {
        
        return ((t1->size() == t2->size() || !t1->size() || !t2->size())
                && t1->getSub() == t2->getSub());
    }
    
    return false;
}

bool operator!=(shared_ptr<ValueType> t1, shared_ptr<ValueType> t2)
{
    return !(t1 == t2);
}
