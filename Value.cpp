#include <utility>
#include <type_traits>
#include "Value.h"

template< class T>
T value_cast (Value* v){
    //using no_ref_type = typename std::remove_reference<T>::type;
    using type = typename std::remove_pointer<T>::type;
    if (type::classDataType() == v->dataType()){
        return static_cast<T>(v);
    }
    return nullptr;
}
template< class T>
T value_cast (const Value* v){
    //using no_ref_type = typename std::remove_reference<T>::type;
    using type = typename std::remove_pointer<T>::type;
    if (type::classDataType() == v->dataType()){
        return static_cast<T>(v);
    }
    return nullptr;
}

//template RealValue* value_cast<RealValue>(Value*);
//template IntegerValue* value_cast<IntegerValue>(Value*);
template TextValue* value_cast<TextValue*>(Value*);
template IntegerValue* value_cast<IntegerValue*>(Value*);
template RealValue* value_cast<RealValue*>(Value*);
template const TextValue* value_cast<const TextValue*>(Value*);
template const IntegerValue* value_cast<const IntegerValue*>(Value*);
template const RealValue* value_cast<const RealValue*>(Value*);
template const TextValue* value_cast<const TextValue*>(const Value*);
template const IntegerValue* value_cast<const IntegerValue*>(const Value*);
template const RealValue* value_cast<const RealValue*>(const Value*);
