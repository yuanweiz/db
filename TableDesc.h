#ifndef __TABLEDESC_H
#define __TABLEDESC_H
#include <string>
#include <memory>
#include <vector>
#include "Types.h"
class TableColumn{
public:
    TableColumn(StorageClass storage,const std::string& name)
        :storage_(storage),
        type_(storageClassToDataType(storage)),
        name_(name)
    {
    }
    const std::string& name()const{return name_;}
    DataType type()const{return type_;}
    StorageClass storageClass()const{return storage_;}
private:
    StorageClass storage_;
    DataType type_;
    std::string name_;
};

std::unique_ptr<TableColumn> textColumn(const std::string& name);
std::unique_ptr<TableColumn> integerColumn(const std::string& name);
std::unique_ptr<TableColumn> doubleColumn(const std::string& name);

class TableDesc{
private:
    using TableColumnPtr = std::unique_ptr<TableColumn>;
    using ColumnList = std::vector<std::unique_ptr<TableColumn>>;
    template <class stl_iter, class data_type>
    class iterator_type {
        public:
            iterator_type(const stl_iter &iter)
                :iter_(iter)
            {
            }
            iterator_type(const iterator_type&)=default;
            data_type & operator*() const{
                return *iter_->get();
            }
            data_type* operator->() const{
                return iter_->get();
            }
            iterator_type& operator++(){
                ++iter_;
                return *this;
            }
            iterator_type operator++(int){
                iterator bak = *this;
                iter_++;
                return bak;
            }
            bool operator!= (const iterator_type&rhs)const{
                return iter_!=rhs.iter_;
            }
            bool operator== (const iterator_type&rhs)const{
                return iter_==rhs.iter_;
            }
        private:
            stl_iter iter_;
    };
    std::string name_;
    ColumnList columns_;
public:
    TableDesc& operator << (TableColumnPtr&& ptr){
        columns_.emplace_back(std::move(ptr));
        return *this;
    }
    TableDesc(const std::string& tableName)
        :name_(tableName)
    {
    }
    using const_iterator = iterator_type<ColumnList::const_iterator, const TableColumn>;
    using iterator = iterator_type<ColumnList::iterator, TableColumn>;
    const_iterator begin()const{
        return const_iterator(columns_.cbegin());
    }
    const_iterator end()const{
        return const_iterator(columns_.cend());
    }
    iterator begin(){
        return iterator(columns_.begin());
    }
    iterator end(){
        return iterator(columns_.end());
    }
    size_t size()const {return columns_.size();}
};

#endif// __TABLEDESC_H
