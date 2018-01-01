#include <utility>
#include <type_traits>
#include "Value.h"

template< class T>
T value_cast (Value* v){
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
