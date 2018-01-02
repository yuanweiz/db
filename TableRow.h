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
    template <class stl_iter, class data_type>
    class iterator_type{
    public:
        iterator_type(const iterator_type&)=default;
        iterator_type (const stl_iter& iter)
            :iter_(iter){}
        data_type& operator*()const{
            return *(iter_->get());
        }
        data_type* operator ->() const {
            return iter_->get();
        }
        iterator_type& operator++(){
            ++iter_;
            return *this;
        }
        bool operator!=(const iterator_type& rhs){
            return iter_!=rhs.iter_;
        }
        bool operator==(const iterator_type& rhs){
            return iter_==rhs.iter_;
        }
        iterator_type operator++(int){
            return iterator_type(iter_++);
        }
    private:
        stl_iter iter_;
    };
public:
    using const_iterator = iterator_type<ValueList::const_iterator,const Value>;
    using iterator = iterator_type<ValueList::iterator,Value>;
    iterator begin(){
        return iterator(values_.begin());
    }
    iterator end(){
        return iterator(values_.end());
    }
    const_iterator begin()const {
        return const_iterator(values_.cbegin());
    }
    const_iterator end()const {
        return const_iterator(values_.cend());
    }
    size_t size()const {return values_.size();}
    TableRow & operator << (Value*);
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
