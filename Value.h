#ifndef __VALUE_H
#define __VALUE_H
#include <string>
#include <sys/types.h>
#include "Types.h"

class Value{
public:
    virtual ~Value(){}
    virtual Value* clone()const=0;
    DataType dataType()const {
        return dataType_;
    }
protected:
    Value(DataType dt):dataType_(dt){}
    const DataType dataType_;
};


//using IntegerValue = ValueDerived<int64_t,DataType::INTEGER>;
//using RealValue = ValueDerived<double,DataType::REAL>;

class RealValue: public Value{
public:
    RealValue(const RealValue&)=default;
    RealValue(double d):Value(DataType::REAL),value_(d){}
    Value * clone() const override {
        return new RealValue(value_);
    }
    static DataType classDataType(){
        return DataType::REAL;
    }
private:
    double value_;
};

class TextValue: public Value{
public:
    TextValue(const TextValue&)=default;
    TextValue(const std::string& str)
        :Value(DataType::TEXT),
        value_(str)
    {
    }
    Value* clone()const override {
        return new TextValue(*this);
    }
    static DataType classDataType(){
        return DataType::TEXT;
    }
private:
    std::string value_;
};

class IntegerValue :public Value{
public:
    IntegerValue(const IntegerValue&)=default;
    IntegerValue(int64_t i)
        :Value(DataType::INTEGER),
        value_(i)
    {
    }
    Value* clone()const override {
        return new IntegerValue(value_);
    }
    static DataType classDataType(){
        return DataType::INTEGER;
    }
private:
    int64_t value_;
};

template <class T> T value_cast(Value*);

#endif // __VALUE_H
