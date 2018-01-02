#ifndef __TABLE_ROW_H
#define __TABLE_ROW_H
#include <memory>
#include <vector>
#include <string>
#include "StringView.h"
class TableDesc;

class Value;

class TableRow{
    using ValueList = std::vector<std::unique_ptr<Value>>;
public:
    class iterator{
    public:
        iterator(const iterator&)=default;
        iterator (const ValueList::iterator& iter)
            :iter_(iter){}
        Value& operator*()const{
            return *(iter_->get());
        }
        Value* operator ->() const {
            return iter_->get();
        }
        iterator& operator++(){
            ++iter_;
            return *this;
        }
        bool operator!=(const iterator& rhs){
            return iter_!=rhs.iter_;
        }
        bool operator==(const iterator& rhs){
            return iter_==rhs.iter_;
        }
        iterator operator++(int){
            return iterator(iter_++);
        }
    private:
        ValueList::iterator iter_;
    };
    iterator begin(){
        return iterator(values_.begin());
    }
    iterator end(){
        return iterator(values_.end());
    }
    TableRow & operator << (const char*);
    TableRow & operator << (int64_t);
    TableRow & operator << (uint64_t);
    TableRow & operator << (int32_t);
    TableRow & operator << (uint32_t);
    TableRow & operator << (int16_t);
    TableRow & operator << (uint16_t);
    TableRow & operator << (bool);
    TableRow & operator << (const std::string&);
    TableRow & operator << (double);
    ~TableRow();
private:
    template <class T> TableRow& addInteger(T);
    ValueList values_;
};
#endif// __TABLE_ROW_H
