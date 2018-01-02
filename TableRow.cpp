#include "TableRow.h"
#include "Value.h"

template <class T> 
TableRow& TableRow::addInteger(T i){
    values_.emplace_back(new IntegerValue(i));
    return *this;
}
TableRow::~TableRow(){
}
TableRow & TableRow::operator<<(const char* str){
    values_.emplace_back(new TextValue(str));
    return *this;
}
TableRow & TableRow::operator<<(const std::string& str){
    values_.emplace_back(new TextValue(str));
    return *this;
}
#define ADD_INT_IMPL(type)\
TableRow & TableRow::operator<<(type i){\
    return addInteger<type>(i);\
}
ADD_INT_IMPL(bool)
ADD_INT_IMPL(uint16_t)
ADD_INT_IMPL(int16_t)
ADD_INT_IMPL(uint32_t)
ADD_INT_IMPL(int32_t)
ADD_INT_IMPL(uint64_t)
ADD_INT_IMPL(int64_t)
#undef OPERATOR_DECL


TableRow & TableRow::operator<<(double d){
    values_.emplace_back(new RealValue(d));
    return *this;
}
TableRow & TableRow::operator<<(Value* v){
    values_.emplace_back(v);
    return *this;
}
    
